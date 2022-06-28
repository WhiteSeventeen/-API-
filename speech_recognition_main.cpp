#ifdef _WIN64
#pragma comment(lib,"./libs/msc_x64.lib")
#else
#pragma comment(lib, "./libs/msc.lib")
#endif

#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable: 4996)


#include <iostream>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>
#include <process.h>

#include "./include/msp_cmn.h"
#include "./include/msp_errors.h"
#include "./include/speech_recognizer.h"
#include "Control.h"

#define FRAME_LEN	640 
#define	BUFFER_SIZE	4096

using namespace std;

enum {
	EVT_START = 0,
	EVT_STOP,
	EVT_QUIT,
	EVT_TOTAL
};
static HANDLE events[EVT_TOTAL] = { NULL,NULL,NULL };

static COORD begin_pos = { 0, 0 };
static COORD last_pos = { 0, 0 };

//显示结果
static void show_result(char* string, char is_over)
{
	COORD orig, current;
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE w = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(w, &info);
	current = info.dwCursorPosition;

	if (current.X == last_pos.X && current.Y == last_pos.Y) {
		SetConsoleCursorPosition(w, begin_pos);
	}
	else {
		/* changed by other routines, use the new pos as start */
		begin_pos = current;
	}
	if (is_over)
		SetConsoleTextAttribute(w, FOREGROUND_GREEN);
	printf("Result: [ %s ]\n", string);

	if (is_over)
		SetConsoleTextAttribute(w, info.wAttributes);

	GetConsoleScreenBufferInfo(w, &info);
	last_pos = info.dwCursorPosition;
}

static void show_key_hints(void)
{
	printf("\n\
----------------------------\n\
输入 r 开始识别\n\
输入 s 停止识别\n\
输入 q 退出\n\
----------------------------\n");
}

//上传同学名称
static int upload_userwords()
{
	char* userwords = NULL;
	size_t			len = 0;
	size_t			read_len = 0;
	FILE* fp = NULL;
	int				ret = -1;

	fp = fopen("test.csv", "rb");
	if (NULL == fp)
	{
		printf("\nopen [test.csv] failed! \n");
		goto upload_exit;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp); //获取文件大小
	fseek(fp, 0, SEEK_SET);

	userwords = (char*)malloc(len + 1);
	if (NULL == userwords)
	{
		printf("\nout of memory! \n");
		goto upload_exit;
	}

	read_len = fread((void*)userwords, 1, len, fp); //读取用户词表内容
	if (read_len != len)
	{
		printf("\nread [test.csv] failed!\n");
		goto upload_exit;
	}
	userwords[len] = '\0';

	MSPUploadData("userwords", userwords, len, "sub = uup, dtt = userword", &ret); //上传用户词表
	if (MSP_SUCCESS != ret)
	{
		printf("\nMSPUploadData failed ! errorCode: %d \n", ret);
		goto upload_exit;
	}

upload_exit:
	if (NULL != fp)
	{
		fclose(fp);
		fp = NULL;
	}
	if (NULL != userwords)
	{
		free(userwords);
		userwords = NULL;
	}

	return ret;
}


//开启线程持续等待输入语音
static unsigned int  __stdcall helper_thread_proc(void* para)
{
	int key;
	int quit = 0;

	do {
		key = _getch();
		switch (key) {
		case 'r':
		case 'R':
			SetEvent(events[EVT_START]);
			break;
		case 's':
		case 'S':
			SetEvent(events[EVT_STOP]);
			break;
		case 'q':
		case 'Q':
			quit = 1;
			SetEvent(events[EVT_QUIT]);
			PostQuitMessage(0);
			break;
		default:
			break;
		}

		if (quit)
			break;
	} while (1);

	return 0;
}

//创建线程
static HANDLE start_helper_thread()
{
	HANDLE hdl;

	hdl = (HANDLE)_beginthreadex(NULL, 0, helper_thread_proc, NULL, 0, NULL);

	return hdl;
}

static char* g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;

void on_result(const char* result, char is_last)
{
	if (result) {
		size_t left = g_buffersize - 1 - strlen(g_result);
		size_t size = strlen(result);
		if (left < size) {
			g_result = (char*)realloc(g_result, g_buffersize + BUFFER_SIZE);
			if (g_result)
				g_buffersize += BUFFER_SIZE;
			else {
				printf("mem alloc failed\n");
				return;
			}
		}
		strncat(g_result, result, size);
		show_result(g_result, is_last);
		string C_identification_result = g_result;
		Do_for_result(C_identification_result);
	}
}

void on_speech_begin()
{
	if (g_result)
	{
		free(g_result);
	}
	g_result = (char*)malloc(BUFFER_SIZE);
	g_buffersize = BUFFER_SIZE;
	memset(g_result, 0, g_buffersize);

	printf("开始识别...\n");
}

void on_speech_end(int reason)
{
	if (reason == END_REASON_VAD_DETECT)
		printf("\nSpeaking done \n");
	else
		printf("\nRecognizer error %d\n", reason);
}


/* demo recognize the audio from microphone */
static void demo_mic(const char* session_begin_params)
{
	int errcode;
	int i = 0;
	HANDLE helper_thread = NULL;

	struct speech_rec iat;
	DWORD waitres;
	char isquit = 0;

	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end
	};

	//初始化
	errcode = sr_init(&iat, session_begin_params, SR_MIC, DEFAULT_INPUT_DEVID, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed\n");
		return;
	}
	//创建事件r/s/q
	for (i = 0; i < EVT_TOTAL; ++i) {
		events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	//创建线程
	helper_thread = start_helper_thread();
	if (helper_thread == NULL) {
		printf("create thread failed\n");
		goto exit;
	}
	//显示指令相关
	show_key_hints();

	while (1) {
		//挂起线程直到输入对应指令
		waitres = WaitForMultipleObjects(EVT_TOTAL, events, FALSE, INFINITE);
		switch (waitres) {
		case WAIT_FAILED:
		case WAIT_TIMEOUT://超时
			printf("Why it happened !?\n");
			break;
		case WAIT_OBJECT_0 + EVT_START://启动
			if (errcode = sr_start_listening(&iat)) {
				printf("start listen failed %d\n", errcode);
				isquit = 1;
			}
			break;
		case WAIT_OBJECT_0 + EVT_STOP://停止
			if (errcode = sr_stop_listening(&iat)) {
				printf("stop listening failed %d\n", errcode);
				isquit = 1;
			}
			break;
		case WAIT_OBJECT_0 + EVT_QUIT://退出
			sr_stop_listening(&iat);
			isquit = 1;
			break;
		default:
			break;
		}
		if (isquit)
			break;
	}
exit:
	if (helper_thread != NULL) {
		WaitForSingleObject(helper_thread, INFINITE);
		CloseHandle(helper_thread);//销毁线程
	}

	for (i = 0; i < EVT_TOTAL; ++i) {
		if (events[i])
			CloseHandle(events[i]);//销毁事件
	}

	sr_uninit(&iat);
}


/* main thread: start/stop record ; query the result of recgonization.
 * record thread: record callback(data write)
 * helper thread: ui(keystroke detection)
 */
int main(int argc, char* argv[])
{
	int			ret = MSP_SUCCESS;
	int			upload_on = 1; //是否上传同学名称
	const char* login_params = "appid = 0c93f281, work_dir = ."; // 登录参数，appid与msc库绑定,请勿随意改动
	int aud_src = 1;
	const char* session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = gb2312";

	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数	
	if (MSP_SUCCESS != ret) {
		printf("MSPLogin failed , Error code %d.\n", ret);
	}

	printf("\n########################################################################\n");
	printf("## 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。##\n");
	printf("########################################################################\n\n");
	printf("演示示例选择:是否上传同学名称？\n0:不使用\n1:使用\n");

	scanf("%d", &upload_on);
	if (upload_on)
	{
		printf("上传同学名称 ...\n");
		ret = upload_userwords();
		printf("上传同学名称成功\n");
	}
	demo_mic(session_begin_params);

	printf("按任意键退出 ...\n");
	_getch();
	MSPLogout(); //退出登录

	return 0;
}
