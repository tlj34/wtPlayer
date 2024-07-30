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

	//��ȡ�����ļ�
	ifstream fp("setting.txt");
	if (!fp.is_open()) {
		cerr << "ERROR: �޷��������ļ�" << endl;
		return 1;
	}
	getline(fp, ffmpegPath);
	fp >> width >> height >> exponential;

	//���ÿ���̨�����ն�����
	DWORD dwMode = 0;
	GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode);
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	//��ȡ��Ƶ����
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

	//��ȡ��Ƶ
	VideoCapture video;
	if (!video.open(videoName)) {
		cerr << "ERROR: �޷�����Ƶ�ļ�" << endl;
		return 1;
	}

	//��ȡ��Ƶ
	string command = ffmpegPath;
	command += " -i \"" + videoName + "\" -f mp3 -vn wtPlayer_tmp.mp3";
	system(command.c_str());

	//������Ƶ
	mciSendString("open wtPlayer_tmp.mp3 alias bkmusic", NULL, 0, NULL);
	mciSendString("play bkmusic", NULL, 0, NULL);

	puts("\33c\n\33[?25l\n\33[30;40m"); //���������ع�겢�����ն���ɫ

	int d, lost = 0, totalFrames = video.get(CAP_PROP_FRAME_COUNT);
	Mat img(width, height, CV_8UC1, cv::Scalar(0, 0, 0));
	Vec3b color, bgColor = { 0, 0, 0 }, txtColor = { 0, 0, 0 };
	auto startTime = high_resolution_clock::now();
	double totalTime = totalFrames / video.get(CAP_PROP_FPS);
	string output;

	//�ж�������ɫ֮��Ĳ����Ƿ񳬹�'d'
	auto f = [&d](Vec3b x, Vec3b y) -> bool {
		return max({ abs(x[0] - y[0]), abs(x[1] - y[1]), abs(x[2] - y[2]) }) > d;
		};

	//��ȡ��ǰʱ��
	auto getTime = [&]() -> double {
		return duration<double>(high_resolution_clock::now() - startTime).count() * video.get(CAP_PROP_FPS);
		};

	//������Ƶ
	for (int idx = 0; idx < totalFrames; ++idx) {
		Mat tmp0, tmp1;
		double current = getTime();
		video.read(tmp0); //��ȡͼ��

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

		//�����������
		for (int i = 0; i < height - 1; i += 2) {
			for (int j = 0; j < width; ++j) {
				if (!f(tmp1.at<Vec3b>(i, j), img.at<Vec3b>(i, j)) && !f(tmp1.at<Vec3b>(i + 1, j), img.at<Vec3b>(i + 1, j))) {
					flag1 = 1;
					continue; //��Ļ�ϵ�ǰλ�õ���ɫ�����ӡ����ɫ���򲻴�ӡ
				}
				if (flag1) {
					flag1 = flag2 = 0;
					output += "\33[" + to_string(i / 2 + 1) + ';' + to_string(j + 1) + 'H'; //�ƶ����
				}
				color = tmp1.at<Vec3b>(i, j);
				if (f(color, bgColor)) { //�ı䱳����ɫ
					output += "\33[48;2;" + to_string(color[2]) + ';' + to_string(color[1]) + ';' + to_string(color[0]) + 'm';
					bgColor = color;
				}
				color = tmp1.at<Vec3b>(i + 1, j);
				if (f(color, txtColor)) { //�ı��ı���ɫ
					output += "\33[38;2;" + to_string(color[2]) + ';' + to_string(color[1]) + ';' + to_string(color[0]) + 'm';
					txtColor = color;
				}
				output += "�{";
			}
			if (!flag1) output += '\n';
		}
		if (!flag2) puts(output.c_str()); //��ӡͼ��
		img = tmp1;
	}

	//ֹͣ����
	mciSendString("stop bkmusic", NULL, 0, NULL);
	mciSendString("close bkmusic", NULL, 0, NULL);

	//ɾ�����ɵ���Ƶ���˳�����
	remove("wtPlayer_tmp.mp3");
	puts("\33[0m\n\33[?25h\n\33c");

	return 0;
}