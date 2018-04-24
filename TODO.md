# BUG:
1. ~~在inspector中，改变text，出现乱码。~~
2. 由于euler角的多种表示方法，输入(0, 180, 0)转成quaternion再转回euler角时，变为(180, 0, 180).
   https://stackoverflow.com/questions/11103683/euler-angle-to-quaternion-then-quaternion-to-euler-angle
3. 导入带light, camera的fbx，使用test_light_camera.fbx.

