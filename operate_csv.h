//�Ʒ�ϵͳ���룬�ο�Դ�룺https://github.com/Routhleck/CSV-memory-words-app
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

//ÿ�ζ�ȡ���ݱ���֮�����������֮����д��
struct STU {
public:
	STU(
		string stu_num,
		string name,
		int score
	) {
		STU_NUM = stu_num;
		NAME = name;
		SCORE = score;
	}
	string STU_NUM;
	string NAME;
	int SCORE;
};

void operation(int flag, string& name)
{
	vector<STU> stus;
	ifstream myFile;
	myFile.open("test.csv", ios::in);
	string line = "";
	getline(myFile, line);
	line = "";
	while (getline(myFile, line))
	{
		string stu_num;
		string name;
		int score;
		string tempString;

		stringstream inputString(line);

		getline(inputString, stu_num, ',');
		getline(inputString, name, ',');
		getline(inputString, tempString, ',');

		score = atoi(tempString.c_str());

		STU Stu(stu_num, name, score);
		stus.push_back(Stu);

		for (int i = 0; i < stus.size(); i++) {
			stus[i].STU_NUM.erase(std::remove(stus[i].STU_NUM.begin(), stus[i].STU_NUM.end(), '\"'), stus[i].STU_NUM.end());
		}

		for (int i = 0; i < stus.size(); i++) {
			stus[i].NAME.erase(std::remove(stus[i].NAME.begin(), stus[i].NAME.end(), '\"'), stus[i].NAME.end());
		}

		line = "";
	}
	myFile.close();

	//��ָ��ͬѧ�ӷ�

	//�Ա�����
	int temp = 0;
	for (int i = 0; i < stus.size(); i++) {
		if (name.compare(stus.at(i).STU_NUM.c_str()))
		{
			temp = i;
			break;
		}
	}

	if (flag == 1)
	{
		stus.at(temp).SCORE++;
	}
	if (flag == -1)
	{
		stus.at(temp).SCORE--;
	}
	ofstream list;
	list.open("test.csv", ios::out);
	list << "ѧ��" << "," << "����" << "," << "����" << endl;
	for (int i = 0; i < stus.size(); i++) {
		list << stus.at(i).STU_NUM << "," << stus.at(i).NAME << "," << stus.at(i).SCORE << endl;
	}
	list.close();
}
