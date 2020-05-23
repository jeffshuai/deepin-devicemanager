#ifndef CMDTOOL_H
#define CMDTOOL_H

#include <QObject>
#include <QMap>
#include <QProcess>
#include <QFile>

class CmdTool
{
public:
    CmdTool();

    static QMap<QString, QList<QMap<QString, QString> > > &getCmdInfo();
    static void clear();
    static void addMapInfo(const QString key, const QMap<QString, QString> &mapInfo);

    /**
     * @brief:通过命令获取设备信息
     * @brief[loadCmdInfo]:一般的处理方式
     * @brief[loadLshwInfo]:lshw命令的处理方式不同
     */
    void loadCmdInfo(const QString &key, const QString &cmd, const QString &debugFile = QString(""));
private:
    void loadLshwInfo(const QString &cmd, const QString &debugFile = QString("lshw.txt"));
    void loadLsblkInfo(const QString &cmd, const QString &debugfile);
    void loadSmartCtlInfo(const QString &cmd, const QString &debugfile);
    void loadXrandrInfo(const QString &cmd, const QString &debugfile);
    void loadXrandrVerboseInfo(const QString &cmd, const QString &debugfile);
    void loadDmesgInfo(const QString &cmd, const QString &debugfile);
    void loadHciconfigInfo(const QString &cmd, const QString &debugfile);
    void loadPrinterInfo();
    void loadHwinfoInfo(const QString &key, const QString &cmd, const QString &debugfile);
    void loadDmidecodeInfo(const QString &key, const QString &cmd, const QString &debugfile);
    void loadCatInfo(const QString &key, const QString &cmd, const QString &debugfile);
    void loadUpowerInfo(const QString &key, const QString &cmd, const QString &debugfile);
    void loadBiosInfoFromLspci(QString &chipsetFamliy);


private:
    /**
     * @brief:将通过命令获取的信息，转化为map形式
     */
    void getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromSmartctl(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch = QString(": "));
    void getMapInfoFromHciconfig(QMap<QString, QString> &mapInfo, const QString &info);

    /**
     * @brief:命令的执行
     */
    bool getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile = QString(""));
    bool executeProcess(const QString &cmd, QString &deviceInfo);
    bool runCmd(const QString &cmd, QString &deviceInfo);

private:
    static QMap<QString, QList<QMap<QString, QString> > > s_cmdInfo;
};

#endif // CMDTOOL_H