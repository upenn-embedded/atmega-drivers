import pyautogui
import time

# Test with simulation https://www.drifted.com/realdrive/

# Multithreading

time.sleep(5)

pyautogui.keyDown('w')
time.sleep(7)
pyautogui.keyUp('w')

pyautogui.keyDown('s')
time.sleep(1)
pyautogui.keyUp('s')
