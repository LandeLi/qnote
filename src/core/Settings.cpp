#include "Settings.h"
#include <QStandardPaths>
#include <QDir>

namespace qnote {

std::unique_ptr<Settings> Settings::s_instance = nullptr;

Settings* Settings::instance() {
    if (!s_instance) {
        s_instance.reset(new Settings());
    }
    return s_instance.get();
}

Settings::Settings() = default;

Settings::~Settings() {
    if (m_settings) {
        m_settings->sync();
    }
}

void Settings::init(const QString& organization, const QString& application) {
    m_settings = std::make_unique<QSettings>(organization, application);
    m_filePath = m_settings->fileName();
}

void Settings::initFromFile(const QString& filePath) {
    m_settings = std::make_unique<QSettings>(filePath, QSettings::IniFormat);
    m_filePath = filePath;
}

QVariant Settings::value(const QString& key, const QVariant& defaultValue) const {
    if (!m_settings) {
        return defaultValue;
    }
    return m_settings->value(key, defaultValue);
}

void Settings::setValue(const QString& key, const QVariant& value) {
    if (m_settings) {
        m_settings->setValue(key, value);
    }
}

bool Settings::contains(const QString& key) const {
    return m_settings && m_settings->contains(key);
}

void Settings::remove(const QString& key) {
    if (m_settings) {
        m_settings->remove(key);
    }
}

void Settings::clear() {
    if (m_settings) {
        m_settings->clear();
    }
}

// 窗口配置
QByteArray Settings::windowGeometry() const {
    return value("window/geometry").toByteArray();
}

void Settings::setWindowGeometry(const QByteArray& geometry) {
    setValue("window/geometry", geometry);
}

QByteArray Settings::windowState() const {
    return value("window/state").toByteArray();
}

void Settings::setWindowState(const QByteArray& state) {
    setValue("window/state", state);
}

// 外观配置
QString Settings::theme() const {
    return value("appearance/theme", "light").toString();
}

void Settings::setTheme(const QString& theme) {
    setValue("appearance/theme", theme);
}

QString Settings::fontFamily() const {
    return value("appearance/fontFamily", "Noto Sans CJK SC").toString();
}

void Settings::setFontFamily(const QString& family) {
    setValue("appearance/fontFamily", family);
}

int Settings::fontSize() const {
    return value("appearance/fontSize", 12).toInt();
}

void Settings::setFontSize(int size) {
    setValue("appearance/fontSize", size);
}

// 编辑器配置
bool Settings::autoSave() const {
    return value("editor/autoSave", true).toBool();
}

void Settings::setAutoSave(bool enabled) {
    setValue("editor/autoSave", enabled);
}

int Settings::autoSaveInterval() const {
    return value("editor/autoSaveInterval", 30).toInt();
}

void Settings::setAutoSaveInterval(int seconds) {
    setValue("editor/autoSaveInterval", seconds);
}

bool Settings::spellCheck() const {
    return value("editor/spellCheck", false).toBool();
}

void Settings::setSpellCheck(bool enabled) {
    setValue("editor/spellCheck", enabled);
}

// 系统配置
bool Settings::minimizeToTray() const {
    return value("system/minimizeToTray", true).toBool();
}

void Settings::setMinimizeToTray(bool enabled) {
    setValue("system/minimizeToTray", enabled);
}

bool Settings::closeToTray() const {
    return value("system/closeToTray", true).toBool();
}

void Settings::setCloseToTray(bool enabled) {
    setValue("system/closeToTray", enabled);
}

bool Settings::startWithSystem() const {
    return value("system/startWithSystem", false).toBool();
}

void Settings::setStartWithSystem(bool enabled) {
    setValue("system/startWithSystem", enabled);
}

// 数据配置
QString Settings::dataPath() const {
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return value("data/path", defaultPath).toString();
}

void Settings::setDataPath(const QString& path) {
    setValue("data/path", path);
}

int Settings::maxBackups() const {
    return value("data/maxBackups", 10).toInt();
}

void Settings::setMaxBackups(int count) {
    setValue("data/maxBackups", count);
}

// 同步配置
bool Settings::syncEnabled() const {
    return value("sync/enabled", false).toBool();
}

void Settings::setSyncEnabled(bool enabled) {
    setValue("sync/enabled", enabled);
}

QString Settings::syncServer() const {
    return value("sync/server").toString();
}

void Settings::setSyncServer(const QString& server) {
    setValue("sync/server", server);
}

QString Settings::filePath() const {
    return m_filePath;
}

void Settings::sync() {
    if (m_settings) {
        m_settings->sync();
    }
}

} // namespace qnote
