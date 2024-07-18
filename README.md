# wtPlayer
这个项目可在Windows10/11的控制台或终端上播放视频。

编译环境：Windows11 + OpenCV 4.8.0

setting.txt中有三行：
第一行为FFmepg.exe的路径（一般在`FFmepg安装路径\bin\FFmepg.exe`）；
第二行有两个整数，表示输出的帧宽度和帧高度，其中帧高度必须是偶数；
第三行有一个是浮点数，理论上数值越大时画面越流畅但越容易出现图像噪声。