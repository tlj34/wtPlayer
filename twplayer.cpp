#include <opencv2/opencv.hpp>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
	//获取宽、高和ffmepg的路径
	int width, height;
	char ffmepgPath[512];
	FILE* fp = fopen("setting.txt", "r");
	fscanf(fp, "%[^\n]", ffmepgPath);
	fscanf(fp, "%d%d\n", &width, &height);

	string videoName = "";
	if (argc < 2) {
		cout << "请输入视频路径：";
		getline(cin, videoName);
	}
	else videoName += argv[1];
	
	if (videoName[0] == '\"') {
		videoName.erase(videoName.end() - 1);
		videoName.erase(videoName.begin());
	}

	VideoCapture video;
	video.open(videoName);

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //获取句柄

	//启用控制台虚拟终端序列
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	int totalFrames = video.get(CAP_PROP_FRAME_COUNT);
	vector<vector<vector<Vec3b>>>color(totalFrames, vector<vector<Vec3b>>(height, vector<Vec3b>(width, { 0, 0, 0 })));

	//加载视频
	cout << "Loading...";
	for (int n = 0; n < totalFrames; ++n) {
		Mat tmp0, tmp1;
		video.read(tmp0);
		resize(tmp0, tmp0, { width, height }, 0, 0);
		cvtColor(tmp0, tmp1, COLOR_BGR2GRAY);

		for (int i = 0; i < tmp0.rows; ++i)
			for (int j = 0; j < tmp0.cols; ++j)
				color[n][i][j] = tmp0.at<Vec3b>(i, j);
	}

	//提取音频
	string command = ffmepgPath;
	command += " -i \"" + videoName + "\" -f mp3 -vn wtPlayer_tmp.mp3";
	system(command.c_str());

	//播放音频
	mciSendString("open wtPlayer_tmp.mp3 alias bkmusic", NULL, 0, NULL);
	mciSendString("play bkmusic", NULL, 0, NULL);

	//清屏并隐藏光标
	system("cls");
	CONSOLE_CURSOR_INFO cursorInfo = { 1, 0 };
	SetConsoleCursorInfo(hOut, &cursorInfo);

	COORD pOs = { 0, 0 };
	Vec3b bgColor = { 0, 0, 0 }, txtColor = { 0, 0, 0 };
	clock_t currentTime = 0;
	clock_t totalTime = totalFrames / video.get(CAP_PROP_FPS) * CLOCKS_PER_SEC;
	clock_t startTime = clock();
	string output;

	//播放视频
	while (1) {
		//获取当前帧
		currentTime = clock() - startTime;
		int current = 1.0 * currentTime / totalTime * totalFrames;
		if (current >= totalFrames) break;

		output.clear();

		//加载输出内容
		for (int i = 0; i < height; i += 2) {
			for (int j = 0; j < width; ++j) {
				Vec3b currentColor = color[current][i][j];
				if (currentColor != bgColor) { //改变背景颜色
					output += "\x1b[48;2;" + to_string(currentColor[2]) + ';' + to_string(currentColor[1]) + ';' + to_string(currentColor[0]) + 'm';
					bgColor = currentColor;
				}

				currentColor = color[current][i + 1][j];
				if (currentColor != txtColor) { //改变文本颜色
					output += "\x1b[38;2;" + to_string(currentColor[2]) + ';' + to_string(currentColor[1]) + ';' + to_string(currentColor[0]) + 'm';
					txtColor = currentColor;
				}
				output += "▄";
			}
			output += '\n';
		}
		SetConsoleCursorPosition(hOut, pOs); //移动光标至控制台左上角
		puts(output.c_str());
	}

	puts("\x1b[0m"); //还原终端颜色

	//停止播放
	mciSendString("stop bkmusic", NULL, 0, NULL);
	mciSendString("close bkmusic", NULL, 0, NULL);

	//删除生成的音频并退出程序
	system("del wtPlayer_tmp.mp3");
	system("cls");
	system("pause");

	return 0;
}