#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "operate_csv.h"

using namespace std;

void Do_for_result(string& s)//nameΪҪs�а�����������sΪ���������
{
	string name;
	int code;
	string namelist[30] = { "�����","������","��֣��","������","��һ��","�ų���",
						"Ԭ����","��Ҷ��","̷˼Զ","ʯ��","�ֳ�Ң","�˼���",
						"���ܺ�","½����","��һ��","������","������","���ݷ�"
						,"������","��־��","������","Ҷ����","������","��Ѿ�"
						,"������","�Ͽ���","��ʩϲ","��ΰҵ" };
	string s_name = "";
	string scode = "";
	int command = 0;
	//��ȡ��s��ǰ�������֣���������Ϊ���������У�
	string temp = "";
	temp = s.substr(0, 4);
	if (temp == "����" || temp == "Ǯ��")
	{
		name = temp;
	}
	else
	{
		name = s.substr(0, 6);
	}
	if (namelist->find(name))
	{
		cout << "�����ڴ���" << endl;
	}
	else cout << "����" << endl;
	//��s�н�ȡ�����롰�ӡ��͡��������Ƚ�
	//���ܱ�ţ�0�����ӡ���1��������
	for (int i = 0; i != s.length(); i += 2)
	{
		string temp = "";
		temp = temp + s[i] + s[i + 1];
		if (temp == "��")
			command = 1;
		else if (temp == "��")
			command = -1;
	}

	if (command == 0) cout << "ʶ��ʧ�ܡ�" << endl;
	else
	{
		operation(command, name);
	}
}
