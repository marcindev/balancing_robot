#include "stackEncoder.h"
#include <sstream>
#include <memory>
#include <algorithm>
#include <dirent.h>
#include <stdio.h>
#include "processExec.h"



using namespace std;

StackEncoder::StackEncoder(bool _isMaster, uint8_t _partition) :
        isMaster(_isMaster),
        partition(_partition)
{
    cout << "Building symbol table from axf file..." << endl;
    isSymTableBuilt = buildSymTableFromAxf();

    if(!isSymTableBuilt)
        cout << "ERR: Couldn't build symbol table!" << endl;
}


void StackEncoder::decodeStacktrace(string& line)
{
    if(!isSymTableBuilt)
        return;

    size_t pos = line.find("Stacktrace:");

    if(pos == string::npos)
        return;

    string decodedStackTr("\n");

    while(true)
    {
        pos = line.find("[", pos);

        if(pos == string::npos)
            break;

        ++pos;
        size_t end = line.find("]", pos);
        size_t len = end - pos;

        string strHexNum = line.substr(pos, len);

        stringstream ss;

        ss << hex << strHexNum;

        uint32_t address;
        ss >> address;

        decodedStackTr += "[" + getFuncName(address) + "]->";
    }

    line += decodedStackTr;
}

string StackEncoder::getFuncName(uint32_t address)
{
    for(const auto& entry : symbolTable)
    {
        if(isAddrInFunc(entry, address))
        {
            return entry.name;
        }
    }

    return string();
}

bool StackEncoder::isAddrInFunc(const SymTableEntry& func, uint32_t address)
{
    return (address >= func.address) && (address < (func.address + func.size));
}

bool StackEncoder::buildSymTableFromAxf()
{
    string strTarget = isMaster ? "master" : "slave";
    string strPartition = (partition == 1) ? "partition_1" : "partition_2";
    string axfPath("./" + strTarget + "/" + strPartition + "/");

    DIR* dirStream;
    dirent *dir;
    dirStream = opendir(axfPath.c_str());

    bool isAxfFound = false;

    if(dirStream)
    {
        while((dir = readdir(dirStream)) != NULL)
        {
            string fileName(dir->d_name);

            if(fileName.find(".axf") != string::npos)
            {
                isAxfFound = true;
                axfPath += fileName;
                break;
            }
        }

        closedir(dirStream);
    }

    if(!isAxfFound)
    {
        cout << "ERR: Couldn't find an .axf file in directory: " << axfPath << endl;
        return false;
    }

    string readelfName("/usr/bin/readelf");

    ProcessExec readelfProc(readelfName);

    vector<string> args;

    args.push_back("-s");
    args.push_back(axfPath);

    readelfProc.setCaptureOutput(true);
    readelfProc.execute(args);

    shared_ptr<const string> output(readelfProc.getOutput());

    stringstream ss(*output);

    string line;

    const unsigned ADDR_COL = 1,
                   SIZE_COL = 2,
                   TYPE_COL = 3,
                   NAME_COL = 7;

    while(getline(ss, line))
    {
        unsigned column = 0;
        stringstream ssLine(line);
        string strElem;

        SymTableEntry symTableEntry;
        bool isFunction = false;

        while(ssLine >> strElem)
        {
            switch(column)
            {
            case ADDR_COL:
            {
                stringstream ssAddr;
                ssAddr << hex << strElem;

                unsigned address;
                ssAddr >> address;
                symTableEntry.address = address;

                break;
            }
            case SIZE_COL:
            {
                stringstream ssSize;
                ssSize << strElem;

                unsigned size;
                ssSize >> size;
                symTableEntry.size = size;

                break;
            }
            case TYPE_COL:
            {
                if(strElem == "FUNC")
                    isFunction = true;

                break;
            }
            case NAME_COL:
            {
                if(isFunction)
                {
                    symTableEntry.name = strElem;
                    symbolTable.push_back(symTableEntry);
                }

                break;
            }
            default:
                break;
            }

            ++column;
        }
    }

    sortSymTable();

    return true;
}

void StackEncoder::sortSymTable()
{
    auto sortByAddr = [](const SymTableEntry& arg1, const SymTableEntry& arg2)->bool {
        return arg1.address < arg2.address;
    };

    sort(symbolTable.begin(), symbolTable.end(), sortByAddr);
}
