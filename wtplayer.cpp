#include <opencv2/opencv.hpp>
#include <windows.h>
#include <fstream>
#undef max

using namespace cv;
using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
	int width, height;
	double exponential;
	string ffmpegPath;

	//读取设置文件
	ifstream fp("setting.txt");
	if (!fp.is_open()) {
		cerr << "ERROR: 无法打开设置文件" << endl;
		return 1;
	}
	getline(fp, ffmpegPath);
	fp >> width >> height >> exponential;

	//启用控制台虚拟终端序列
	DWORD dwMode = 0;
	GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode);
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	//获取视频名称
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

	//获取视频
	VideoCapture video;
	if (!video.open(videoName)) {
		cerr << "ERROR: 无法打开视频文件" << endl;
		return 1;
	}

	//提取音频
	string command = ffmpegPath;
	command += " -i \"" + videoName + "\" -f mp3 -vn wtPlayer_tmp.mp3";
	system(command.c_str());

	//播放音频
	mciSendString("open wtPlayer_tmp.mp3 alias bkmusic", NULL, 0, NULL);
	mciSendString("play bkmusic", NULL, 0, NULL);

	puts("\33c\n\33[?25l\n\33[30;40m"); //清屏、隐藏光标并设置终端颜色

	int d, lost = 0, totalFrames = video.get(CAP_PROP_FRAME_COUNT);
	Mat img(width, height, CV_8UC1, cv::Scalar(0, 0, 0));
	Vec3b color, bgColor = { 0, 0, 0 }, txtColor = { 0, 0, 0 };
	auto startTime = high_resolution_clock::now();
	double totalTime = totalFrames / video.get(CAP_PROP_FPS);
	string output;

	//判断两个颜色之间的差异是否超过'd'
	auto f = [&d](Vec3b x, Vec3b y) -> bool {
		return max({ abs(x[0] - y[0]), abs(x[1] - y[1]), abs(x[2] - y[2]) }) > d;
		};

	//获取当前时间
	auto getTime = [&]() -> double {
		return duration<double>(high_resolution_clock::now() - startTime).count() * video.get(CAP_PROP_FPS);
		};

	//播放视频
	for (int idx = 0; idx < totalFrames; ++idx) {
		Mat tmp0, tmp1;
		double current = getTime();
		video.read(tmp0); //获取图像

		if (current > idx) {
			++lost;
			continue;
		}
		while (current < idx) current = getTime();
		
		bool flag1 = 1, flag2 = 1;
		d = 0.5 + exponential * lost;
		lost = 0;
		output.clear();
		resize(tmp0, tmp1, { width, height });

		//加载输出内容
		for (int i = 0; i < height - 1; i += 2) {
			for (int j = 0; j < width; ++j) {
				if (!f(tmp1.at<Vec3b>(i, j), img.at<Vec3b>(i, j)) && !f(tmp1.at<Vec3b>(i + 1, j), img.at<Vec3b>(i + 1, j))) {
					flag1 = 1;
					continue; //屏幕上当前位置的颜色和需打印的颜色相则不打印
				}
				if (flag1) {
					flag1 = flag2 = 0;
					output += "\33[" + to_string(i / 2 + 1) + ';' + to_string(j + 1) + 'H'; //移动光标
				}
				color = tmp1.at<Vec3b>(i, j);
				if (f(color, bgColor)) { //改变背景颜色
					output += "\33[48;2;" + to_string(color[2]) + ';' + to_string(color[1]) + ';' + to_string(color[0]) + 'm';
					bgColor = color;
				}
				color = tmp1.at<Vec3b>(i + 1, j);
				if (f(color, txtColor)) { //改变文本颜色
					output += "\33[38;2;" + to_string(color[2]) + ';' + to_string(color[1]) + ';' + to_string(color[0]) + 'm';
					txtColor = color;
				}
				output += "▄";
			}
			if (!flag1) output += '\n';
		}
		if (!flag2) puts(output.c_str()); //打印图像
		img = tmp1;
	}

	//停止播放
	mciSendString("stop bkmusic", NULL, 0, NULL);
	mciSendString("close bkmusic", NULL, 0, NULL);

	//删除生成的音频并退出程序
	remove("wtPlayer_tmp.mp3");
	puts("\33[0m\n\33[?25h\n\33c");

	return 0;
}