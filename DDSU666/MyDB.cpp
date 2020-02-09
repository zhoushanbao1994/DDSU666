/*************************************************************************
> File Name: MyDB.cpp
************************************************************************/

#include <iostream>
#include <string>
#include <stdio.h>
#include "MyDB.h"
#include "easylogging++.h"

using namespace std;

MyDB::MyDB()
{
    m_mysql = mysql_init(NULL);   //初始化数据库连接变量
    if(m_mysql == NULL)
    {
        LOG(INFO) << "Error:" << mysql_error(m_mysql);
        exit(1);
    }
}

MyDB::~MyDB()
{
    if(m_mysql != NULL)  //关闭数据连接
    {
        mysql_close(m_mysql);
    }
}


bool MyDB::initDB(char* host, char* user, char* passwd, char* db_name)
{
    // 函数mysql_real_connect建立一个数据库连接  
    // 成功返回MYSQL*连接句柄，失败返回NULL  
    m_mysql = mysql_real_connect(m_mysql, host, user, passwd, NULL, 0, NULL, 0); 
    if(m_mysql == NULL)  
    {  
        LOG(INFO) << "MySQL Connect Error: " << mysql_error(m_mysql);  
        exit(1);  
    }
    else {
        LOG(INFO) << "MySQL Connect Success!";
    }
    
    char strSql[128];
    sprintf(strSql, "CREATE DATABASE IF NOT EXISTS %s", db_name);
    // 创建数据库
    if(mysql_query(m_mysql, strSql)) {
        LOG(INFO) << "MySQL Create Database Error:" << mysql_error(m_mysql);
        exit(1);
    }
    else {
        LOG(INFO) << "MySQL Create Database Success!";
    }
    return true;  
}

bool MyDB::connectDB(char* host, char* user, char* passwd, char* db_name)//连接mysql
{
    // 函数mysql_real_connect建立一个数据库连接  
    // 成功返回MYSQL*连接句柄，失败返回NULL  
    m_mysql = mysql_real_connect(m_mysql, host, user, passwd, db_name, 0, NULL, 0); 
    if(m_mysql == NULL) {  
        LOG(INFO) << "MySQL Connect Error: " << mysql_error(m_mysql);  
        exit(1);  
    }
    else {
        LOG(INFO) << "MySQL Connect Success!";
    }
    
    return true;
}

bool MyDB::exeSQL(char* sql)
{
    MYSQL_RES *result;    //指向查询结果的指针
    MYSQL_ROW row;       //按行返回的查询信息

    int  num_fields;  //获取结果集中总共的字段数，即列数
    int  num_rows;
    
    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(m_mysql, sql))
    {
        LOG(INFO) << "MySQL Query Error: " << mysql_error(m_mysql);
        return false;
    }
    else // 查询成功
    {
        result = mysql_store_result(m_mysql);  //获取结果集
        if (result)  // 返回了结果集
        {
           num_fields = mysql_num_fields(result);   //获取结果集中总共的字段数，即列数
           num_rows = mysql_num_rows(result);       //获取结果集中总共的行数
           for(int i = 0; i < num_rows; i++) //输出每一行
            {
                //获取下一行数据
                row = mysql_fetch_row(result);
                if(row < 0) break;

                for(int j = 0; j < num_fields; j++)  //输出每一字段
                {
                    cout << row[j] << "\t\t";
                }
                cout << endl;
            }
        }
        else  // result==NULL
        {
            if(mysql_field_count(m_mysql) == 0)   //代表执行的是update,insert,delete类的非查询语句
            {
                // (it was not a SELECT)
                num_rows = mysql_affected_rows(m_mysql);  //返回update,insert,delete影响的行数
            }
            else // error
            {
                LOG(INFO) << "MySQL Get result error: " << mysql_error(m_mysql);;
                return false;
            }
        }
    }

    return true;

}


bool MyDB::connectBatteryDataTable(char* tableName) // 创建表
{
    char strSql[512];
    sprintf(strSql, "CREATE TABLE IF NOT EXISTS `%s` ( \
 `DateTime` DATETIME NOT NULL, \
 `Voltage(V)` FLOAT NOT NULL, \
 `Current(A)` FLOAT NOT NULL, \
 `ActivePower(kW)` FLOAT NOT NULL, \
 `ReactivePower(kW)` FLOAT NOT NULL, \
 `ApparentPower(VA)` FLOAT NOT NULL, \
 `PowerFactor` FLOAT NOT NULL, \
 `Frequency(Hz)` FLOAT NOT NULL, \
 PRIMARY KEY ( `datetime` ) \
 )ENGINE=InnoDB DEFAULT CHARSET=utf8;", tableName);
 
    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(m_mysql, strSql)) {
        LOG(INFO) << "MySQL Query Error: " << mysql_error(m_mysql);;
        return false;
    }
    else {
        LOG(INFO) << "MySQL Connect Battery DataTable Success!";;
    }
    return true;
}

bool MyDB::connectEnergyDataTable(char* tableName) // 创建电能表
{
    char strSql[512];
    sprintf(strSql, "CREATE TABLE IF NOT EXISTS `%s` ( \
 `DateTime` DATETIME NOT NULL, \
 `ImpEp(kWh)` FLOAT NOT NULL, \
 PRIMARY KEY ( `datetime` ) \
 )ENGINE=InnoDB DEFAULT CHARSET=utf8;", tableName);
 
    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(m_mysql, strSql)) {
        LOG(INFO) << "MySQL Query Error: " << mysql_error(m_mysql);
        return false;
    }
    else {
        LOG(INFO) << "MySQL Connect Battery DataTable Success!";
    }
    return true;
}

bool MyDB::addDate(char* tableName, float fValtage, float fCurrent, 
            float fActivePower, float fReactivePower, float fApparentPower,
            float fPowerFactor, float fFrequency)// 增加数据
{
    char strSql[512];
    
    // 基于当前系统的当前日期/时间
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char timeBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", ltm);
    
    sprintf(strSql, "INSERT `%s` VALUES('%s','%.03f','%.03f','%.03f','%.03f','%.03f','%.03f','%.03f');",
        tableName,
        timeBuf,
        fValtage,
        fCurrent, 
        fActivePower,
        fReactivePower,
        fApparentPower,
        fPowerFactor,
        fFrequency);
    
    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(m_mysql, strSql)) {
        LOG(INFO) << "MySQL Query Error: " << mysql_error(m_mysql);
        return false;
    }
    else {
        LOG(INFO) << "MySQL Add Data Success!";
    }
    return true;
}

bool MyDB::addDate(char* tableName, float fImpEp)// 增加数据
{
    char strSql[512];
    
    // 基于当前系统的当前日期/时间
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char timeBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", ltm);
    
    sprintf(strSql, "INSERT `%s` VALUES('%s','%.03f');",
        tableName,
        timeBuf,
        fImpEp);
    
    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(m_mysql, strSql)) {
        LOG(INFO) << "MySQL Query Error: " << mysql_error(m_mysql);
        return false;
    }
    else {
        LOG(INFO) << "MySQL Add Data Success!";
    }
    return true;
}


