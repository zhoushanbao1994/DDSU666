/*************************************************************************
    > File Name: mydb.h
 ************************************************************************/

#ifndef _MYDB_H
#define _MYDB_H
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <stdlib.h>

using namespace std;

class MyDB
{
public:
    MyDB();
    ~MyDB();
    bool initDB(char* host, char* user, char* pwd, char* db_name);  // 初始化数据库
    bool connectDB(char* host, char* user, char* pwd, char* db_name);//连接mysql
    bool exeSQL(char* sql);   //执行sql语句
    bool connectBatteryDataTable(char* tableName); // 创建电量表
    bool connectEnergyDataTable(char* tableName); // 创建电能表
    bool addDate(char* tableName, float fValtage, float fCurrent, 
            float fActivePower, float fReactivePower, float fApparentPower,
            float fPowerFactor, float fFrequency); // 增加数据
    bool addDate(char* tableName, float fImpEp); // 增加数据

private:
    MYSQL *m_mysql;          //连接mysql句柄指针
};


#endif
