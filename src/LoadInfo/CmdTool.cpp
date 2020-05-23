#include "CmdTool.h"
#include <cups.h>
#include "commondefine.h"
#include<QDebug>
extern void showDetailedInfo(cups_dest_t *dest, const char *option, QMap<QString, QString> &DeviceInfoMap);
extern int getDestInfo(void *user_data, unsigned flags, cups_dest_t *dest);

QMap<QString, QList<QMap<QString, QString> > > CmdTool::s_cmdInfo;

CmdTool::CmdTool()
{

}

QMap<QString, QList<QMap<QString, QString> > > &CmdTool::getCmdInfo()
{
    return s_cmdInfo;
}
void CmdTool::clear()
{
    s_cmdInfo.clear();
}

void CmdTool::addMapInfo(const QString key, const QMap<QString, QString> &mapInfo)
{
    if (s_cmdInfo.find(key) != s_cmdInfo.end()) {
        s_cmdInfo[key].append(mapInfo);
    } else {
        QList<QMap<QString, QString> > lstMap;
        lstMap.append(mapInfo);
        s_cmdInfo.insert(key, lstMap);
    }
}

void CmdTool::loadCmdInfo(const QString &key, const QString &cmd, const QString &debugFile)
{
    if (key == "lshw") {
        loadLshwInfo(cmd, debugFile);
    } else if (key == "lsblk_d") {
        loadLsblkInfo(cmd, debugFile);
    } else if (key == "xrandr") {
        loadXrandrInfo(cmd, debugFile);
    } else if (key == "xrandr_verbose") {
        loadXrandrVerboseInfo(cmd, debugFile);
    } else if (key == "dmesg") {
        loadDmesgInfo(cmd, debugFile);
    } else if (key == "hciconfig") {
        loadHciconfigInfo(cmd, debugFile);
    } else if (key == "printer") {
        loadPrinterInfo();
    } else if (key == "upower") {
        loadUpowerInfo(key, cmd, debugFile);
    } else if (key.startsWith("hwinfo")) {
        loadHwinfoInfo(key, cmd, debugFile);
    } else if (key.startsWith("dmidecode")) {
        loadDmidecodeInfo(key, cmd, debugFile);
    } else {
        loadCatInfo(key, cmd, debugFile);
    }
}

void CmdTool::loadLshwInfo(const QString &cmd, const QString &debugFile)
{
    QString deviceInfo;
    getDeviceInfo(cmd, deviceInfo, debugFile);
    QStringList items = deviceInfo.split("*-");
    bool isFirst = true;
    foreach (const QString &item, items) {
        QMap<QString, QString> mapInfo;
        if (isFirst) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_system", mapInfo);
            isFirst = false;
            continue;
        }
        if (item.startsWith("cpu")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_cpu", mapInfo);
        } else if (item.startsWith("disk")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_disk", mapInfo);
        } else if ((item.startsWith("memory") && !item.startsWith("memory UNCLAIMED")) || item.startsWith("bank:")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_memory", mapInfo);
        } else if (item.startsWith("display")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_display", mapInfo);
        } else if (item.startsWith("multimedia")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_multimedia", mapInfo);
        } else if (item.startsWith("network")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_network", mapInfo);
        } else if (item.startsWith("usb:")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_usb", mapInfo);
        }
    }
}

void CmdTool::loadLsblkInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }
    QStringList lines = deviceInfo.split("\n");
    QMap<QString, QString> mapInfo;
    foreach (QString line, lines) {
        QStringList words = line.replace(QRegExp("[\\s]+"), " ").split(" ");
        if (words.size() != 2 || words[0] == "NAME") {
            continue;
        }
        mapInfo.insert(words[0].trimmed(), words[1].trimmed());

        //****************
        QString smartCmd = QString("sudo smartctl --all /dev/%1").arg(words[0].trimmed());
        loadSmartCtlInfo(smartCmd, words[0].trimmed());
    }
    addMapInfo("lsblk_d", mapInfo);
}

void CmdTool::loadSmartCtlInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }
    QMap<QString, QString> mapInfo;
    mapInfo["ln"] = debugfile;
    getMapInfoFromSmartctl(mapInfo, deviceInfo);
    addMapInfo("smart", mapInfo);
}

void CmdTool::loadXrandrInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }

    QMap<QString, QString> mapInfo;
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
//        QRegExp reline("[a-zA-Z]{1,3}.*");
//        if (!reline.exactMatch(line)) {
//            continue;
//        }
        QRegExp reResolution("^[\\s]{3}([0-9]{3,5}x[0-9]{3,5}).*([0-9]{2,3}.[0-9]{2,3}\\*).*");
        if (reResolution.exactMatch(line)) {
            //QString resolution = reResolution.cap(1);
            QString rate = reResolution.cap(2).replace("*", "");
            mapInfo.insert("rate", rate);
        }
        if (line.startsWith("Screen")) {
            QRegExp re(".*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*");
            if (re.exactMatch(line)) {
                mapInfo["minResolution"] = re.cap(1);
                mapInfo["curResolution"] = re.cap(2);
                mapInfo["maxResolution"] = re.cap(3);
            }
        } else if (line.startsWith("HDMI")) {
            mapInfo["HDMI"] = "Enable";
        } else if (line.startsWith("VGA")) {
            mapInfo["VGA"] = "Enable";
        } else if (line.startsWith("DP")) {
            mapInfo["DP"] = "Enable";
        } else if (line.startsWith("eDP")) {
            mapInfo["eDP"] = "Enable";
        }
    }
    addMapInfo("xrandr", mapInfo);
}

void CmdTool::loadXrandrVerboseInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(QString(cmd), deviceInfo, debugfile)) {
        return;
    }

    QStringList lines = deviceInfo.split(QRegExp("\n"));
    QString mainInfo("");
    QString edid("");
    foreach (QString line, lines) {
        if (line.startsWith("Screen")) {
            continue;
        }
        QRegExp reResolution("^[\\s]{2}([0-9]{3,4}x[0-9]{3,4}).*");
        if (reResolution.exactMatch(line)) {
            continue;
        }
        QRegExp reMain("^[a-zA-Z].*");
        if (reMain.exactMatch(line)) {
            if (!mainInfo.isEmpty()) {
                QMap<QString, QString> mapInfo;
                mapInfo.insert("mainInfo", mainInfo.trimmed());
                mapInfo.insert("edid", edid.trimmed());
                addMapInfo("xrandr_verbose", mapInfo);
            }
            mainInfo = line;
            edid = "";
            continue;
        }
        QRegExp reEdid("^[\\t]{2}[0-9]{1}.*");
        if (reEdid.exactMatch(line)) {
            edid.append(line.trimmed());
            edid.append("\n");
            continue;
        }
    }
    QMap<QString, QString> mapInfo;
    mapInfo.insert("mainInfo", mainInfo.trimmed());
    mapInfo.insert("edid", edid.trimmed());
    addMapInfo("xrandr_verbose", mapInfo);
}

void CmdTool::loadDmesgInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }
    QMap<QString, QString> mapInfo;
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        QRegExp reg(".*RAM=([0-9]*)M.*");
        if (reg.exactMatch(line)) {
            double size = reg.cap(1).toDouble();
            QString sizeS = QString("%1GB").arg(size / 1024);
            mapInfo["Size"] = sizeS;
        } else {
            reg.setPattern(".*RAM: ([0-9]*) M.*");
            if (reg.exactMatch(line)) {
                double size = reg.cap(1).toDouble();
                QString sizeS = QString("%1GB").arg(size / 1024);
                mapInfo["Size"] = sizeS;
            }
        }
    }
    addMapInfo("dmesg", mapInfo);
}

void CmdTool::loadHciconfigInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }
    QStringList paragraphs = deviceInfo.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        QMap<QString, QString> mapInfo;
        getMapInfoFromHciconfig(mapInfo, paragraph);
        addMapInfo("hciconfig", mapInfo);
    }
}

//void showDetailedInfo(cups_dest_t *dest, const char *option, QMap<QString, QString> &DeviceInfoMap)
//{
//    cups_dinfo_t *info = cupsCopyDestInfo(CUPS_HTTP_DEFAULT, dest);

//    if (true == cupsCheckDestSupported(CUPS_HTTP_DEFAULT, dest, info, option, nullptr)) {
//        ipp_attribute_t *finishings = cupsFindDestSupported(CUPS_HTTP_DEFAULT, dest, info, option);
//        int i, count = ippGetCount(finishings);

//        //std::cout << option << " support" << std::endl;
//        for (i = 0; i < count; i ++) {
//            if (option == CUPS_FINISHINGS || option == CUPS_COPIES /*|| option == CUPS_MEDIA */ || \
//                    option == CUPS_NUMBER_UP || option == CUPS_ORIENTATION || option == CUPS_PRINT_QUALITY) {
//                if (DeviceInfoMap.contains(option)) {
//                    DeviceInfoMap[option] += ", ";
//                    DeviceInfoMap[option] += QString::number(ippGetInteger(finishings, i));
//                } else {
//                    DeviceInfoMap[option] = QString::number(ippGetInteger(finishings, i));
//                }

//                //std::cout << option<<" : "<<ippGetInteger(finishings, i)  << std::endl;
//            } else {
//                if (DeviceInfoMap.contains(option)) {
//                    DeviceInfoMap[option] += ", ";
//                    DeviceInfoMap[option] += ippGetString(finishings, i, nullptr);
//                } else {
//                    DeviceInfoMap[option] = ippGetString(finishings, i, nullptr);
//                }

//                //std::cout << option<<" : " << ippGetString(finishings, i, nullptr) << std::endl;
//            }
//        }
//    } else {
//        //std::cout << option << " not supported." << std::endl;
//    }
//}
////获取相关打印机目标字段的信息
//int getDestInfo(void *user_data, unsigned flags, cups_dest_t *dest)
//{
//    if (flags & CUPS_DEST_FLAGS_REMOVED) {
//        return -1;
//    }

//    QMap<QString, QMap<QString, QString> > &cupsDatabase = *(reinterpret_cast<QMap<QString, QMap<QString, QString> > *>(user_data));

//    if (dest == nullptr) {
//        return -1;
//    }

//    QMap<QString, QString> DeviceInfoMap;

//    showDetailedInfo(dest, CUPS_FINISHINGS, DeviceInfoMap);
//    showDetailedInfo(dest, CUPS_COPIES, DeviceInfoMap);
//    showDetailedInfo(dest, CUPS_MEDIA, DeviceInfoMap);
//    showDetailedInfo(dest, CUPS_MEDIA_SOURCE, DeviceInfoMap);
//    showDetailedInfo(dest, CUPS_MEDIA_TYPE, DeviceInfoMap);
//    showDetailedInfo(dest, CUPS_NUMBER_UP, DeviceInfoMap);
//    showDetailedInfo(dest, CUPS_ORIENTATION, DeviceInfoMap);
//    showDetailedInfo(dest, CUPS_PRINT_COLOR_MODE, DeviceInfoMap);
//    showDetailedInfo(dest, CUPS_PRINT_QUALITY, DeviceInfoMap);
//    showDetailedInfo(dest, CUPS_SIDES, DeviceInfoMap);


//    //cups_dinfo_t* info = cupsCopyDestInfo(CUPS_HTTP_DEFAULT, dest);

//    for (int i = 0; i < dest->num_options; ++i) {
//        DeviceInfoMap[(dest->options + i)->name] = (dest->options + i)->value;
//        //std::cout << (dest->options + i)->name <<" : " << (dest->options + i)->value << std::endl;
//    }

//    cupsDatabase[dest->name] = DeviceInfoMap;

//    return (1);
//}
void CmdTool::loadPrinterInfo()
{
    QMap<QString, QMap<QString, QString> > info;
    //获得所有的句柄
    cupsEnumDests(CUPS_DEST_FLAGS_NONE, 1000, nullptr, CUPS_PRINTER_CLASS, 0, getDestInfo, &info);

    foreach (const QString &key, info.keys()) {
        info[key]["Name"] = key;
        addMapInfo("printer", info[key]);
    }
}

void CmdTool::loadHwinfoInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    getDeviceInfo(cmd, deviceInfo, debugfile);
    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        getMapInfoFromHwinfo(item, mapInfo);

        //预防有相同的总线信息
        if (key == "hwinfo_usb") {
            QList<QMap<QString, QString>>::iterator it = s_cmdInfo[key].begin();
            bool add = true;
            for (; it != s_cmdInfo[key].end(); ++it) {
                QString curBus = (*it)["SysFS BusID"];
                QString newBus = mapInfo["SysFS BusID"];
                curBus.replace(QRegExp("\\.[0-9]{1,2}$"), "");
                newBus.replace(QRegExp("\\.[0-9]{1,2}$"), "");
                if (curBus == newBus) {
                    add = false;
                    break;
                }
            }
            if (add) {
                addMapInfo(key, mapInfo);
            }
        } else {
            addMapInfo(key, mapInfo);
        }
    }
}
void CmdTool::loadDmidecodeInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    getDeviceInfo(cmd, deviceInfo, debugfile);
    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        getMapInfoFromDmidecode(item, mapInfo);
        if (key == "dmidecode2") {
            QString chipset;
            loadBiosInfoFromLspci(chipset);
            mapInfo.insert("chipset", chipset);
        }
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadCatInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }
    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        if (key == "cat_version") {
            mapInfo["OS"] = item;
        } else {
            getMapInfoFromCmd(item, mapInfo, key.startsWith("cat_os") ? "=" : ": ");
        }
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadUpowerInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }
    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty() || item.contains("DisplayDevice")) {
            continue;
        }
        QMap<QString, QString> mapInfo;
        getMapInfoFromCmd(item, mapInfo);
        if (item.contains("Daemon:")) {
            addMapInfo("Daemon", mapInfo);
        } else {
            addMapInfo(key, mapInfo);
        }
    }
}

void CmdTool::loadBiosInfoFromLspci(QString &chipsetFamliy)
{
    // 获取设备信息
    QString command;
    QString deviceInfo;
    if (!getDeviceInfo(QString("lspci"), deviceInfo, "lspci.txt")) {
        return;
    }
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        QStringList words = line.split(" ");
        if (words.size() < 2) {
            continue;
        }
        if (words[1] == QString("ISA")) {
            command = QString("lspci -v -s %1").arg(words[0].trimmed());
            break;
        }
    }

    if (command.isEmpty()) {
        return;
    }
    QString chipset;
    if (!getDeviceInfo(command, chipset, "")) {
        return;
    }
    lines = chipset.split("\n");
    foreach (const QString &line, lines) {
        if (line.contains("Subsystem")) {
            QStringList words = line.split(": ");
            if (words.size() == 2) {
                chipsetFamliy = words[1].trimmed();
            }
            break;
        }
    }

}

void CmdTool::getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() == 2) {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}
void CmdTool::getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() != 2) {
            continue;
        }
        // && words[0].contains("configuration") == false && words[0].contains("resources") == false
        // 将configuration的内容进行拆分
        if (words[0].contains("configuration") == true) {
            QStringList keyValues = words[1].split(" ");

            for (QStringList::iterator it = keyValues.begin(); it != keyValues.end(); ++it) {
                QStringList attr = (*it).split("=");
                if (attr.size() != 2) {
                    continue;
                }
                mapInfo.insert(attr[0].trimmed(), attr[1].trimmed());
            }
        } else if (words[0].contains("resources") == true) {
            QStringList keyValues = words[1].split(" ");

            for (QStringList::iterator it = keyValues.begin(); it != keyValues.end(); ++it) {
                QStringList attr = (*it).split(":");
                if (attr.size() != 2) {
                    continue;
                }
                if (mapInfo.find(attr[0].trimmed()) != mapInfo.end()) {
                    mapInfo[attr[0].trimmed()] += QString("  ");
                }
                mapInfo[attr[0].trimmed()] += attr[1].trimmed();
            }
        } else {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}
void CmdTool::getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() != 2) {
            continue;
        }
        if (mapInfo.find(words[0].trimmed()) != mapInfo.end()) {
            mapInfo[words[0].trimmed()] += QString(" ");
        }
        QRegExp re(".*\"(.*)\".*");
        if (re.exactMatch(words[1].trimmed())) {
            mapInfo[words[0].trimmed()] += re.cap(1);
        } else {
            if (words[0].trimmed() == "Resolution") {
                mapInfo[words[0].trimmed()] += words[1].trimmed();
            } else {
                mapInfo[words[0].trimmed()] = words[1].trimmed();
            }
        }
    }
}
void CmdTool::getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList lines = info.split("\n");
    QString lasKey;
    foreach (const QString &line, lines) {
        if (line.isEmpty()) {
            continue;
        }
        QStringList words = line.split(ch);
        if (words.size() == 1 && words[0].endsWith(":")) {
            lasKey = words[0].replace(QRegExp(":$"), "");
            mapInfo.insert(lasKey.trimmed(), " ");
        } else if (words.size() == 1 && !lasKey.isEmpty()) {
            mapInfo[lasKey.trimmed()] += words[0];
            mapInfo[lasKey.trimmed()] += "  /  ";
        } else if (words.size() == 2) {
            lasKey = "";
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}

void CmdTool::getMapInfoFromSmartctl(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch)
{
    QString indexName;
    int startIndex = 0;

    QRegExp reg("^[\\s\\S]*[\\d]:[\\d][\\s\\S]*$");//time 08:00

    for (int i = 0; i < info.size(); ++i) {
        if (info[i] != '\n' && i != info.size() - 1) {
            continue;
        }

        QString line = info.mid(startIndex, i - startIndex);
        startIndex = i + 1;


        int index = line.indexOf(ch);
        if (index > 0 && reg.exactMatch(line) == false && false == line.contains("Error") && false == line.contains("hh:mm:SS")) {
            if (line.indexOf("(") < index && line.indexOf(")") > index) {
                continue;
            }

            if (line.indexOf("[") < index && line.indexOf("]") > index) {
                continue;
            }

            indexName = line.mid(0, index).trimmed().remove(" is");
            if (mapInfo.contains(indexName)) {
                mapInfo[indexName] += ", ";
                mapInfo[indexName] += line.mid(index + 1).trimmed();
            } else {
                mapInfo[indexName] = line.mid(index + 1).trimmed();
            }
            continue;
        }

        if (indexName.isEmpty() == false && (line.startsWith("\t\t") || line.startsWith("    ")) && line.contains(":") == false) {
            if (mapInfo.contains(indexName)) {
                mapInfo[indexName] += ", ";
                mapInfo[indexName] += line.trimmed();
            } else {
                mapInfo[indexName] = line.trimmed();
            }

            continue;
        }

        indexName = "";

        QRegExp rx("^[ ]*[0-9]+[ ]+([\\w-_]+)[ ]+0x[0-9a-fA-F-]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+([0-9\\/w-]+[ ]*[ 0-9\\/w-()]*)$");
        if (rx.indexIn(line) > -1) {
            mapInfo[rx.cap(1)] = rx.cap(2);
            continue;
        }

        QStringList strList;

        if (line.endsWith(")")) {
            int leftBracket = line.indexOf("(");
            if (leftBracket > 0) {
                QString str = line.left(leftBracket).trimmed();
                strList = str.trimmed().split(" ");
                if (strList.size() > 2) {
                    strList.last() += line.mid(leftBracket);
                }
            }
        } else if (strList.size() == 0) {
            strList = line.trimmed().split(" ");
        }

        if (strList.size() < 5) {
            continue;
        }

        if (line.contains("Power_On_Hours")) {
            mapInfo["Power_On_Hours"] = strList.last();
            continue;
        }

        if (line.contains("Power_Cycle_Count")) {
            mapInfo["Power_Cycle_Count"] = strList.last();
            continue;
        }

        if (line.contains("Raw_Read_Error_Rate")) {
            mapInfo["Raw_Read_Error_Rate"] = strList.last();
            continue;
        }

        if (line.contains("Spin_Up_Time")) {
            mapInfo["Spin_Up_Time"] = strList.last();
            continue;
        }

        if (line.contains("Start_Stop_Count")) {
            mapInfo["Start_Stop_Count"] = strList.last();
            continue;
        }

        if (line.contains("Reallocated_Sector_Ct")) {
            mapInfo["Reallocated_Sector_Ct"] = strList.last();
            continue;
        }

        if (line.contains("Seek_Error_Rate")) {
            mapInfo["Seek_Error_Rate"] = strList.last();
            continue;
        }

        if (line.contains("Spin_Retry_Count")) {
            mapInfo["Spin_Retry_Count"] = strList.last();
            continue;
        }
        if (line.contains("Calibration_Retry_Count")) {
            mapInfo["Calibration_Retry_Count"] = strList.last();
            continue;
        }
        if (line.contains("G-Sense_Error_Rate")) {
            mapInfo["G-Sense_Error_Rate"] = strList.last();
            continue;
        }
        if (line.contains("Power-Off_Retract_Count")) {
            mapInfo["Power-Off_Retract_Count"] = strList.last();
            continue;
        }
        if (line.contains("Load_Cycle_Count")) {
            mapInfo["Load_Cycle_Count"] = strList.last();
            continue;
        }
        if (line.contains("Temperature_Celsius")) {
            mapInfo["Temperature_Celsius"] = strList.last();
            continue;
        }
        if (line.contains("Reallocated_Event_Count")) {
            mapInfo["Reallocated_Event_Count"] = strList.last();
            continue;
        }
        if (line.contains("Current_Pending_Sector")) {
            mapInfo["Current_Pending_Sector"] = strList.last();
            continue;
        }
        if (line.contains("Offline_Uncorrectable")) {
            mapInfo["Offline_Uncorrectable"] = strList.last();
            continue;
        }
        if (line.contains("UDMA_CRC_Error_Count")) {
            mapInfo["UDMA_CRC_Error_Count"] = strList.last();
            continue;
        }
        if (line.contains("Multi_Zone_Error_Rate")) {
            mapInfo["Multi_Zone_Error_Rate"] = strList.last();
            continue;
        }
    }
}

void CmdTool::getMapInfoFromHciconfig(QMap<QString, QString> &mapInfo, const QString &info)
{
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        QStringList pairs = line.trimmed().split("  ");
        if (pairs.size() < 1) {
            continue;
        }
        foreach (const QString &pair, pairs) {
            QStringList keyValue = pair.trimmed().split(": ");
            if (keyValue.size() == 2) {
                mapInfo[keyValue[0].trimmed()] = keyValue[1].trimmed();
            }
        }
    }
}

bool CmdTool::getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile)
{
    if (!deviceInfo.isEmpty()) {
        return true;
    }
//    qint64 begin = QDateTime::currentMSecsSinceEpoch();
    if (false == executeProcess(command, deviceInfo)) {
        return false;
    }
//    qint64 end = QDateTime::currentMSecsSinceEpoch();
//    qDebug() << command << " ******************************* " << (end - begin) / 1000.0;
#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();
#endif

    return true;
}
bool CmdTool::executeProcess(const QString &cmd, QString &deviceInfo)
{
#ifdef TEST_DATA_FROM_FILE
    return true;
#endif
    if (false == cmd.startsWith("sudo")) {
        return runCmd(cmd, deviceInfo);
    }

    runCmd("id -un", deviceInfo);
    if (deviceInfo.trimmed() == "root") {
        return runCmd(cmd, deviceInfo);
    }

    QString newCmd = "pkexec deepin-devicemanager-authenticateProxy \"";
    newCmd += cmd;
    newCmd += "\"";
    newCmd.remove("sudo");
    return runCmd(newCmd, deviceInfo);
}
bool CmdTool::runCmd(const QString &proxy, QString &deviceInfo)
{
    QString key = "devicemanager";
    QString cmd = proxy;
    QProcess process_;
    int msecs = 10000;
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy")) {
        cmd = proxy + QString(" ") + key;
        msecs = -1;
    }

    process_.start(cmd);

    bool res = process_.waitForFinished(msecs);
    deviceInfo = process_.readAllStandardOutput();
    int exitCode = process_.exitCode();
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy") && (exitCode == 127 || exitCode == 126)) {
        //dError("Run \'" + cmd + "\' failed: Password Error! " + QString::number(exitCode) + "\n");
        return false;
    }

    if (res == false) {
        //dError("Run \'" + cmd + "\' failed\n");
    }

    return res;
}