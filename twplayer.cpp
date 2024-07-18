#include <opencv2/opencv.hpp>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
	//��ȡ���ߺ�ffmepg��·��
	int width, height;
	char ffmepgPath[512];
	FILE* fp = fopen("setting.txt", "r");
	fscanf(fp, "%[^\n]", ffmepgPath);
	fscanf(fp, "%d%d\n", &width, &height);

	string videoName = "";
	if (argc < 2) {
		cout << "��������Ƶ·����";
		getline(cin, videoName);
	}
	else videoName += argv[1];
	
	if (videoName[0] == '\"') {
		videoName.erase(videoName.end() - 1);
		videoName.erase(videoName.begin());
	}

	VideoCapture video;
	video.open(videoName);

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //��ȡ���

	//���ÿ���̨�����ն�����
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	int totalFrames = video.get(CAP_PROP_FRAME_COUNT);
	vector<vector<vector<Vec3b>>>color(totalFrames, vector<vector<Vec3b>>(height, vector<Vec3b>(width, { 0, 0, 0 })));

	//������Ƶ
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

	//��ȡ��Ƶ
	string command = ffmepgPath;
	command += " -i \"" + videoName + "\" -f mp3 -vn wtPlayer_tmp.mp3";
	system(command.c_str());

	//������Ƶ
	mciSendString("open wtPlayer_tmp.mp3 alias bkmusic", NULL, 0, NULL);
	mciSendString("play bkmusic", NULL, 0, NULL);

	//���������ع��
	system("cls");
	CONSOLE_CURSOR_INFO cursorInfo = { 1, 0 };
	SetConsoleCursorInfo(hOut, &cursorInfo);

	COORD pOs = { 0, 0 };
	Vec3b bgColor = { 0, 0, 0 }, txtColor = { 0, 0, 0 };
	clock_t currentTime = 0;
	clock_t totalTime = totalFrames / video.get(CAP_PROP_FPS) * CLOCKS_PER_SEC;
	clock_t startTime = clock();
	string output;

	//������Ƶ
	while (1) {
		//��ȡ��ǰ֡
		currentTime = clock() - startTime;
		int current = 1.0 * currentTime / totalTime * totalFrames;
		if (current >= totalFrames) break;

		output.clear();

		//�����������
		for (int i = 0; i < height; i += 2) {
			for (int j = 0; j < width; ++j) {
				Vec3b currentColor = color[current][i][j];
				if (currentColor != bgColor) { //�ı䱳����ɫ
					output += "\x1b[48;2;" + to_string(currentColor[2]) + ';' + to_string(currentColor[1]) + ';' + to_string(currentColor[0]) + 'm';
					bgColor = currentColor;
				}

				currentColor = color[current][i + 1][j];
				if (currentColor != txtColor) { //�ı��ı���ɫ
					output += "\x1b[38;2;" + to_string(currentColor[2]) + ';' + to_string(currentColor[1]) + ';' + to_string(currentColor[0]) + 'm';
					txtColor = currentColor;
				}
				output += "�{";
			}
			output += '\n';
		}
		SetConsoleCursorPosition(hOut, pOs); //�ƶ����������̨���Ͻ�
		puts(output.c_str());
	}

	puts("\x1b[0m"); //��ԭ�ն���ɫ

	//ֹͣ����
	mciSendString("stop bkmusic", NULL, 0, NULL);
	mciSendString("close bkmusic", NULL, 0, NULL);

	//ɾ�����ɵ���Ƶ���˳�����
	system("del wtPlayer_tmp.mp3");
	system("cls");
	system("pause");

	return 0;
}