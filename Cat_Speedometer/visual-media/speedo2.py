#!/usr/bin/env python3
import time, json, math, os
from datetime import datetime, date

import cv2
from picamera2 import Picamera2

from luma.core.interface.serial import i2c
from luma.oled.device import sh1106, ssd1306
from PIL import Image, ImageDraw, ImageFont

from collections import deque

# ----------------------------
# USER CONFIG
# ----------------------------

# Wheel diameter used for speed conversion (inches)
WHEEL_DIAMETER_IN = 43.0

# Number of markers/strips around the wheel
MARKERS_PER_REV = 4

# ROI "gate" where the marker passes (tune these)
ROI_X = 240
ROI_Y = 230
ROI_W = 160
ROI_H = 20

# Detection threshold: higher = needs brighter marker
BRIGHTNESS_THRESHOLD = 100

# Debounce/cooldown so one pass counts once (seconds)
PASS_COOLDOWN_S = 0.35

# If no pass in this long, treat speed as 0
STOP_TIMEOUT_S = 2.0

# Smooth display with EMA
RPM_EMA_ALPHA = 0.35  # 0..1 (higher reacts faster)

# OLED I2C address (common: 0x3C)
OLED_I2C_ADDR = 0x3C

# Try this driver first. If your display is SH1106-based, change to 'sh1106'
OLED_DRIVER = "sh1106"  # "ssd1306" or "sh1106"

STATS_PATH = "wheel_stats.json"

# OLED update rate (seconds). Slower = fewer I2C writes = fewer wedges.
OLED_UPDATE_S = 1.0

# ----------------------------
# Helpers
# ----------------------------

def iso_week_key(d: date) -> str:
    y, w, _ = d.isocalendar()
    return f"{y}-W{w:02d}"

def today_key(d: date) -> str:
    return d.strftime("%Y-%m-%d")

def rpm_to_mph(rpm: float) -> float:
    # mph = rpm * circumference_in * 60 / inches_per_mile
    return rpm * (math.pi * WHEEL_DIAMETER_IN) * 60.0 / 63360.0

def load_stats():
    if os.path.exists(STATS_PATH):
        try:
            with open(STATS_PATH, "r") as f:
                return json.load(f)
        except Exception:
            pass
    # defaults
    now = date.today()
    return {
        "all_time_max_rpm": 0.0,
        "day_key": today_key(now),
        "day_max_rpm": 0.0,
        "week_key": iso_week_key(now),
        "week_max_rpm": 0.0,
    }

def save_stats(stats):
    tmp = STATS_PATH + ".tmp"
    with open(tmp, "w") as f:
        json.dump(stats, f, indent=2)
    os.replace(tmp, STATS_PATH)

def rollover_if_needed(stats):
    now = date.today()
    dk = today_key(now)
    wk = iso_week_key(now)

    changed = False
    if stats.get("day_key") != dk:
        stats["day_key"] = dk
        stats["day_max_rpm"] = 0.0
        changed = True
    if stats.get("week_key") != wk:
        stats["week_key"] = wk
        stats["week_max_rpm"] = 0.0
        changed = True
    return changed

def setup_oled():
    serial = i2c(port=1, address=OLED_I2C_ADDR)
    if OLED_DRIVER.lower() == "sh1106":
        dev = sh1106(serial)
    else:
        dev = ssd1306(serial)
    return dev

def draw_oled(dev, rpm, mph, stats) -> bool:
    """
    Returns True if display update succeeded, False if it failed (I2C error).
    """
    # 1-bit image for OLED
    img = Image.new("1", (dev.width, dev.height), 0)
    draw = ImageDraw.Draw(img)
    draw.rectangle((0, 0, dev.width, dev.height), fill=0)

    # Fonts
    try:
        big = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 38)
        small = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 10)
    except Exception:
        big = ImageFont.load_default()
        small = ImageFont.load_default()

    W, H = dev.width, dev.height

    # Bottom records line
    day_mph  = rpm_to_mph(stats.get("day_max_rpm", 0.0))
    week_mph = rpm_to_mph(stats.get("week_max_rpm", 0.0))
    all_mph  = rpm_to_mph(stats.get("all_time_max_rpm", 0.0))
    records = f"D {day_mph:.2f}  W {week_mph:.2f}  A {all_mph:.2f}"

    bottom_h = 12
    draw.rectangle((0, H - bottom_h, W, H), fill=0)
    draw.text((0, H - bottom_h), records, font=small, fill=255)

    # Big MPH
    mph_str = f"{mph:.1f}"
    bbox = draw.textbbox((0, 0), mph_str, font=big)
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]

    avail_h = H - bottom_h
    x = max(0, (W - tw) // 2)
    y = max(0, (avail_h - th) // 2 - 2)
    draw.text((x, y), mph_str, font=big, fill=255)

    # Try to push the frame to the OLED
    try:
        dev.display(img)
        return True
    except Exception as e:
        # This is the key: don't die, and don't keep spamming I2C if it's wedged.
        print(f"\n[OLED] display update failed: {type(e).__name__}: {e}\n")
        return False

def main():
    stats = load_stats()

    # Ensure required keys exist even if stats file is old/corrupt
    defaults = load_stats()
    for k, v in defaults.items():
        stats.setdefault(k, v)

    if rollover_if_needed(stats):
        save_stats(stats)

    prev_marker_present = False

    # Frame Counter
    frames_on = 0
    frames_off = 0

    # OLED
    dev = setup_oled()
    _ = draw_oled(dev, 0.0, 0.0, stats)

    # Camera
    picam2 = Picamera2()
    config = picam2.create_preview_configuration(main={"format": "BGR888", "size": (640, 480)})
    picam2.configure(config)
    picam2.start()
    time.sleep(1.0)

    last_pass_t = None
    last_trigger_t = 0.0
    rpm_ema = 0.0

    print("Running. Press Ctrl+C to quit.")
    print("Tune ROI_X/Y/W/H and BRIGHTNESS_THRESHOLD if needed.")

    last_oled_update = 0.0
    last_save = time.time()
    prev_frame_t = time.time()

    t_prev = time.perf_counter()
    fps_hist = deque(maxlen=30)
    fps_avg = 0.0

    # Heartbeat so we can tell if the loop is alive
    last_heartbeat = time.time()

    while True:
        frame = picam2.capture_array()
        frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

        # ROI gate
        x, y, w, h = ROI_X, ROI_Y, ROI_W, ROI_H
        roi = frame[y:y+h, x:x+w]

        gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
        mean_brightness = float(gray.mean())

        now = time.time()

        # Heartbeat dot once per second
        if now - last_heartbeat >= 1.0:
            print(".", end="", flush=True)
            last_heartbeat = now

        t_now = time.perf_counter()
        dtp = t_now - t_prev
        t_prev = t_now
        if dtp > 0:
            fps_hist.append(1.0 / dtp)
            fps_avg = sum(fps_hist) / len(fps_hist)

        dt_frame = now - prev_frame_t
        prev_frame_t = now

        # Detect rising edge (marker entering gate) with cooldown
        marker_present = mean_brightness >= BRIGHTNESS_THRESHOLD
        rising_edge = marker_present and (not prev_marker_present)

        if marker_present:
            frames_on += 1
            frames_off = 0
        else:
            frames_off += 1
            if frames_on > 0:
                # Optional debug
                # print(f"\nmarker lasted {frames_on} frames")
                frames_on = 0

        if rising_edge and (now - last_trigger_t) >= PASS_COOLDOWN_S:
            last_trigger_t = now
            if last_pass_t is not None:
                dt = now - last_pass_t
                # Sanity filter for false triggers (scaled for markers)
                if dt > (0.08 / MARKERS_PER_REV):
                    rpm_inst = 60.0 / (dt * MARKERS_PER_REV)
                    rpm_ema = (RPM_EMA_ALPHA * rpm_inst) + ((1.0 - RPM_EMA_ALPHA) * rpm_ema)

                    changed = False
                    if rollover_if_needed(stats):
                        changed = True

                    if rpm_ema > stats["day_max_rpm"]:
                        stats["day_max_rpm"] = rpm_ema
                        changed = True
                    if rpm_ema > stats["week_max_rpm"]:
                        stats["week_max_rpm"] = rpm_ema
                        changed = True
                    if rpm_ema > stats["all_time_max_rpm"]:
                        stats["all_time_max_rpm"] = rpm_ema
                        changed = True

                    if changed:
                        save_stats(stats)

            last_pass_t = now
            print(f"\nPASS  mean={mean_brightness:.1f}  rpm={rpm_ema:.2f}  mph={rpm_to_mph(rpm_ema):.2f}")
            # print(f"roi_shape={roi.shape} mean={mean_brightness:.1f} thresh={BRIGHTNESS_THRESHOLD}")

        # If stopped (no passes), decay to 0
        if last_pass_t is None or (now - last_pass_t) > STOP_TIMEOUT_S:
            rpm_ema *= 0.90
            if rpm_ema < 0.05:
                rpm_ema = 0.0

        prev_marker_present = marker_present

        # Update OLED slower, and auto-recover if I2C glitches
        if (now - last_oled_update) > OLED_UPDATE_S:
            last_oled_update = now
            ok = draw_oled(dev, rpm_ema, rpm_to_mph(rpm_ema), stats)
            if not ok:
                # Give the bus a moment, then re-init the OLED and try once
                time.sleep(0.2)
                try:
                    dev = setup_oled()
                    _ = draw_oled(dev, rpm_ema, rpm_to_mph(rpm_ema), stats)
                    print("[OLED] re-init attempted\n")
                except Exception as e:
                    print(f"[OLED] re-init failed: {type(e).__name__}: {e}\n")

        # Periodic save
        if (time.time() - last_save) > 30.0:
            last_save = time.time()
            save_stats(stats)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nExiting.")