![Desktop Pattern Matcher](images/icon.png) Desktop Pattern Matcher (DPM)
=========

DPM is Qt-based GUI aplication that will let you choose image pattern from your screen and then 
can periodically check your screen for such pattern and click on it if you desire.

This was developed for hunting golden cookies in Cookie Clicker (: http://orteil.dashnet.org/cookieclicker/

## How to install DPM

### Windows
1. Install Microsoft Visual C++ 2017 Redistributable Runtime (for x64)
2. Just download latest release zip archive, unpack it and run executable

### Linux
TODO - if anyone will request such thing

## How to use DPM

1. Click "Set Image From ScreenShot" to grab screenshot and select pattern from it.
   Zooming in and out on screenshot is available. Use crop mode to select pattern.
   Another way is to create pattern.png image in application directory or replace existing one.
2. Set desired check interval (how often should desktop be checked for patterns) and enable
   checking process with disabled "click on matches" checkbox
3. Watch load in the upper right corner. Red means that it takes too long for pattern matching 
   process to finish (longer than check interval).
   You can either make check interval longer or fiddle with threshold 1 and 2. Or both.
   The lower the thresholds, the better the performance but fewer potential matches (yellow ones) will be selected.
4. After getting potential matches right, it's time to make green matches work (real ones).
   By default, exact matching is performed. If needed, tolerant matching can be enabled.
   Try setting as low threshold as possible while still matching desired patterns.
5. Enable "click on matches" checkbox to have DPM clicked in the center of matched patterns 
   (displayed as green crosses in green rectangles).
   
Happy matching (:

![right before clicking](images/dpm.png)
Positive match right before clicking
