#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "operate_csv.h"

using namespace std;

void Do_for_result(string& s)//name为要s中包含的姓名，s为传入的文字
{
	string name;
	int code;
	string namelist[30] = { "冯高云","王喻骢","吴郑扬","褚子宁","何一可","杜承泽",
						"袁漪云","陈叶兰","谭思远","石瑞康","贾成尧","顾佳宇",
						"赵哲浩","陆建高","郭一凡","王成旭","汪鼎昌","米逸凡"
						,"李梦涛","喻志威","许子卿","叶欣宇","王祥雨","李佳静"
						,"王凯乐","邢柯宇","白施喜","张伟业" };
	string s_name = "";
	string scode = "";
	int command = 0;
	//截取出s中前三个汉字（二字姓名为少数，单列）
	string temp = "";
	temp = s.substr(0, 4);
	if (temp == "闫宇" || temp == "钱杰")
	{
		name = temp;
	}
	else
	{
		name = s.substr(0, 6);
	}
	if (namelist->find(name))
	{
		cout << "不存在此人" << endl;
	}
	else cout << "存在" << endl;
	//从s中截取汉字与“加”和“减”作比较
	//功能编号：0代表“加”，1代表“减”
	for (int i = 0; i != s.length(); i += 2)
	{
		string temp = "";
		temp = temp + s[i] + s[i + 1];
		if (temp == "加")
			command = 1;
		else if (temp == "减")
			command = -1;
	}

	if (command == 0) cout << "识别失败。" << endl;
	else
	{
		operation(command, name);
	}
}
