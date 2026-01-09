/*  v4l2ucp - A universal control panel for all V4L2 devices
    Copyright (C) 2005,2009 Scott J. Bertin (scottbertin@yahoo.com)
    Copyright (C) 2009-2010 Vasily Khoruzhick (anarsoul@gmail.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <libv4l2.h>
#include <sys/ioctl.h>

#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QTimer>

#include "mainWindow.h"
#include "previewSettings.h"
#include "v4l2controls.h"

MainWindow::MainWindow(QWidget *parent, const char *name) : QMainWindow(parent), fd(-1), previewProcess(NULL) {
    setWindowTitle(name);
    setWindowIcon(QIcon(":/v4l2ucp.png"));
    QMenu *menu = new QMenu(this);
    menu->addAction("&Open", Qt::CTRL | Qt::Key_O, [this] { this->fileOpen(); });
    menu->addAction("&Close", Qt::CTRL | Qt::Key_W, [this] { this->close(); });
    menu->addSeparator();
    menu->addAction("E&xit", Qt::CTRL | Qt::Key_Q, [] { qApp->exit(); });
    menu->setTitle("&File");
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    resetAllId = menu->addAction("&All");
    resetMenu = menu;
    menu->setTitle("&Reset");
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    updateActions[0] = menu->addAction("Disabled", this, &MainWindow::updateDisabled);
    menu->addSeparator();
    updateActions[1] = menu->addAction("1 sec", this, &MainWindow::update1Sec);
    updateActions[2] = menu->addAction("5 sec", this, &MainWindow::update5Sec);
    updateActions[3] = menu->addAction("10 sec", this, &MainWindow::update10Sec);
    updateActions[4] = menu->addAction("20 sec", this, &MainWindow::update20Sec);
    updateActions[5] = menu->addAction("30 sec", this, &MainWindow::update30Sec);
    menu->addSeparator();
    menu->addAction("Update now", this, &MainWindow::timerShot);
    menu->setTitle("&Update");
    menuBar()->addMenu(menu);
    for (int i = 0; i < 6; i++) {
        updateActions[i]->setCheckable(true);
    }
    updateActions[0]->setChecked(true);

    menu = new QMenu(this);
    menu->addAction("Configure preview...", this, &MainWindow::configurePreview);
    menu->addAction("Start preview", this, &MainWindow::startPreview);
    menu->setTitle("Preview");
    menuBar()->addMenu(menu);

    menu = new QMenu(this);
    menu->addAction("&About", this, &MainWindow::about);
    menu->addAction("About &Qt", this, &MainWindow::aboutQt);
    menu->setTitle("&Help");
    menuBar()->addMenu(menu);

    QObject::connect(&timer, &QTimer::timeout, this, &MainWindow::timerShot);
}

void MainWindow::fileOpen() {
    QFileDialog *diag = new QFileDialog(this, "Select V4L2 device", "/dev",
                                        "V4L2 Devices (video* vout* vbi* radio*);;"
                                        "Video Capture (video*);;"
                                        "Video Output (vout*);;"
                                        "VBI (vbi*);;"
                                        "Radio (radio*);;"
                                        "All Files (*)");

    diag->setFilter(QDir::AllEntries | QDir::System);

    connect(diag, &QFileDialog::fileSelected, [diag](const QString &newfilename) {
        diag->close();
        if (!newfilename.isEmpty()) {
            MainWindow *w = openFile(newfilename.toUtf8());
            if (w)
                w->show();
        }
    });

    diag->show();
}

MainWindow *MainWindow::openFile(const char *fileName) {
    int fd = v4l2_open(fileName, O_RDWR, 0);
    if (fd < 0) {
        QString msg = QString("Unable to open file %1\n%2").arg(fileName, strerror(errno));
        QMessageBox::warning(NULL, "v4l2ucp: Unable to open file", msg);
        return NULL;
    }

    struct v4l2_capability cap;
    if (v4l2_ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        QString msg = QString("%1 is not a V4L2 device").arg(fileName);
        QMessageBox::warning(NULL, "v4l2ucp: Not a V4L2 device", msg);
        return NULL;
    }

    MainWindow *mw = new MainWindow();
    mw->filename = QString(fileName);
    mw->fd = fd;
    QString str("v4l2ucp - ");
    str.append(fileName);
    mw->setWindowTitle(str);

    QScrollArea *sa = new QScrollArea();
    sa->setWidgetResizable(true);

    QWidget *grid = new QWidget(sa);
    sa->setWidget(grid);

    QGridLayout *gridLayout = new QGridLayout(grid);
    grid->setLayout(gridLayout);

    QLabel *l = new QLabel("driver", grid);
    gridLayout->addWidget(l, 0, 0);
    l = new QLabel((const char *)cap.driver, grid);
    gridLayout->addWidget(l, 0, 1);
    l = new QLabel(grid);
    gridLayout->addWidget(l, 0, 2);
    l = new QLabel(grid);
    gridLayout->addWidget(l, 0, 3);

    l = new QLabel("card", grid);
    gridLayout->addWidget(l);
    l = new QLabel((const char *)cap.card, grid);
    gridLayout->addWidget(l);
    l = new QLabel(grid);
    gridLayout->addWidget(l);
    l = new QLabel(grid);
    gridLayout->addWidget(l);

    l = new QLabel("bus_info", grid);
    gridLayout->addWidget(l);
    l = new QLabel((const char *)cap.bus_info, grid);
    gridLayout->addWidget(l);
    l = new QLabel(grid);
    gridLayout->addWidget(l);
    l = new QLabel(grid);
    gridLayout->addWidget(l);

    l = new QLabel("version", grid);
    gridLayout->addWidget(l);
    l = new QLabel(QString("%1.%2.%3").arg(cap.version >> 16).arg((cap.version >> 8) & 0xff).arg(cap.version & 0xff),
                   grid);
    gridLayout->addWidget(l);
    l = new QLabel(grid);
    gridLayout->addWidget(l);
    l = new QLabel(grid);
    gridLayout->addWidget(l);

    l = new QLabel("capabilities", grid);
    gridLayout->addWidget(l);
    l = new QLabel(QString("0x%1").arg(QString::number(cap.capabilities, 16)), grid);
    gridLayout->addWidget(l);
    l = new QLabel(grid);
    gridLayout->addWidget(l);
    l = new QLabel(grid);
    gridLayout->addWidget(l);

    struct v4l2_queryctrl ctrl;
#ifdef V4L2_CTRL_FLAG_NEXT_CTRL
    /* Try the extended control API first */
    ctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
    if (0 == v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
        do {
            mw->add_control(ctrl, fd, grid, gridLayout);
            ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        } while (0 == v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl));
    } else
#endif
    {
        /* Fall back on the standard API */
        /* Check all the standard controls */
        for (int i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {
            ctrl.id = i;
            if (v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
                mw->add_control(ctrl, fd, grid, gridLayout);
            }
        }

        /* Check any custom controls */
        for (int i = V4L2_CID_PRIVATE_BASE;; i++) {
            ctrl.id = i;
            if (v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
                mw->add_control(ctrl, fd, grid, gridLayout);
            } else {
                break;
            }
        }
    }

    mw->setCentralWidget(sa);
    mw->setVisible(true);
    return mw;
}

MainWindow::~MainWindow() {
    if (fd >= 0)
        v4l2_close(fd);
}

void MainWindow::add_control(struct v4l2_queryctrl &ctrl, int fd, QWidget *parent, QGridLayout *layout) {
    QWidget *w = NULL;

    if (ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        return;

    QLabel *l = new QLabel((const char *)ctrl.name, parent);
    layout->addWidget(l);

    switch (ctrl.type) {
    case V4L2_CTRL_TYPE_INTEGER:
        w = new V4L2IntegerControl(fd, ctrl, parent, this);
        break;
    case V4L2_CTRL_TYPE_BOOLEAN:
        w = new V4L2BooleanControl(fd, ctrl, parent, this);
        break;
    case V4L2_CTRL_TYPE_MENU:
        w = new V4L2MenuControl(fd, ctrl, parent, this);
        break;
    case V4L2_CTRL_TYPE_BUTTON:
        w = new V4L2ButtonControl(fd, ctrl, parent, this);
        break;
    case V4L2_CTRL_TYPE_CTRL_CLASS:
        layout->addWidget(new QLabel(parent));
        layout->addWidget(new QLabel(parent));
        layout->addWidget(new QLabel(parent));
        l->setTextFormat(Qt::RichText);
        l->setText(QString("<b>%1</b>").arg((const char *)ctrl.name));
        return;
    case V4L2_CTRL_TYPE_INTEGER64:
    default:
        break;
    }

    if (!w) {
        layout->addWidget(new QLabel("Unknown control", parent));
        layout->addWidget(new QLabel(parent));
        layout->addWidget(new QLabel(parent));
        return;
    }

    layout->addWidget(w);
    if (ctrl.flags & (V4L2_CTRL_FLAG_GRABBED | V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_INACTIVE)) {
        w->setEnabled(false);
    }

    QPushButton *pb;
    pb = new QPushButton("Update", parent);
    layout->addWidget(pb);
    QObject::connect(pb, SIGNAL(clicked()), w, SLOT(updateStatus()));
    QObject::connect(this, SIGNAL(updateNow()), w, SLOT(updateStatus()));

    if (ctrl.type == V4L2_CTRL_TYPE_BUTTON) {
        l = new QLabel(parent);
        layout->addWidget(l);
    } else {
        pb = new QPushButton("Reset", parent);
        layout->addWidget(pb);
        QObject::connect(pb, SIGNAL(clicked()), w, SLOT(resetToDefault()));
        QObject::connect(resetAllId, SIGNAL(triggered(bool)), w, SLOT(resetToDefault()));
    }
}

void MainWindow::about() {
    QMessageBox::about(this, "About",
                       "v4l2ucp Version " V4L2UCP_VERSION "\n\n"
                       "This application is a port of an original v4l2ucp to Qt6 library,\n"
                       "v4l2ucp is a universal control panel for all V4L2 devices. The\n"
                       "controls come directly from the driver. If they cause problems\n"
                       "with your hardware, please contact the maintainer of the driver.\n\n"
                       "Copyright (C) 2005 Scott J. Bertin\n"
                       "Copyright (C) 2009-2010 Vasily Khoruzhick\n\n"
                       "This program is free software; you can redistribute it and/or modify\n"
                       "it under the terms of the GNU General Public License as published by\n"
                       "the Free Software Foundation; either version 2 of the License, or\n"
                       "(at your option) any later version.\n\n"
                       "This program is distributed in the hope that it will be useful,\n"
                       "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                       "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                       "GNU General Public License for more details.\n\n"
                       "You should have received a copy of the GNU General Public License\n"
                       "along with this program; if not, write to the Free Software\n"
                       "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  "
                       "USA\n");
}

void MainWindow::aboutQt() { QMessageBox::aboutQt(this); }

void MainWindow::updateDisabled() {
    for (int i = 0; i < 6; i++) {
        updateActions[i]->setChecked(false);
    }
    updateActions[0]->setChecked(true);
    timer.stop();
}

void MainWindow::update1Sec() {
    for (int i = 0; i < 6; i++) {
        updateActions[i]->setChecked(false);
    }
    updateActions[1]->setChecked(true);
    timer.stop();
    timer.setInterval(1000);
    timer.start();
}

void MainWindow::update5Sec() {
    for (int i = 0; i < 6; i++) {
        updateActions[i]->setChecked(false);
    }
    updateActions[2]->setChecked(true);
    timer.stop();
    timer.setInterval(5000);
    timer.start();
}

void MainWindow::update10Sec() {
    for (int i = 0; i < 6; i++) {
        updateActions[i]->setChecked(false);
    }
    updateActions[3]->setChecked(true);
    timer.stop();
    timer.setInterval(10000);
    timer.start();
}

void MainWindow::update20Sec() {
    for (int i = 0; i < 6; i++) {
        updateActions[i]->setChecked(false);
    }
    updateActions[4]->setChecked(true);
    timer.stop();
    timer.setInterval(20000);
    timer.start();
}

void MainWindow::update30Sec() {
    for (int i = 0; i < 6; i++) {
        updateActions[i]->setChecked(false);
    }
    updateActions[5]->setChecked(true);
    timer.stop();
    timer.setInterval(30000);
    timer.start();
}

void MainWindow::timerShot() { emit(updateNow()); }

void MainWindow::startPreview() {
    if (previewProcess && previewProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(NULL, "v4l2ucp: warning", "Preview process is already started");
        return;
    }

    if (!previewProcess) {
        previewProcess = new QProcess(this);
        QObject::connect(previewProcess, SIGNAL(error(QProcess::ProcessError)), this,
                         SLOT(previewProcError(QProcess::ProcessError)));
        QObject::connect(previewProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                         SLOT(previewFinished(int, QProcess::ExitStatus)));
    }

    QStringList possiblePlayers = {"mpv", "mplayer", "ffplay"};
    QString appBinaryName = "mpv";
    for (const QString &players : possiblePlayers) {
        if (!QStandardPaths::findExecutable(players).isEmpty()) {
            appBinaryName = players;
            break;
        }
    }

    QSettings settings(APP_ORG, APP_NAME);
    if (settings.contains(SETTINGS_APP_BINARY_NAME)) {
        appBinaryName = settings.value(SETTINGS_APP_BINARY_NAME).toString();
    }

    QStringList env = QProcess::systemEnvironment();
    if (settings.contains(SETTINGS_ENV_LIST)) {
        QList<QVariant> envList = settings.value(SETTINGS_ENV_LIST).toList();
        QList<QVariant>::iterator begin, end;
        for (begin = envList.begin(), end = envList.end(); begin != end; begin++) {
            env << (*begin).toString();
        }
    } else {
        // env << "LD_PRELOAD=/usr/lib/libv4l/v4l2convert.so";
    }

    QStringList args;
    if (settings.contains(SETTINGS_ARG_LIST)) {
        QList<QVariant> argList = settings.value(SETTINGS_ARG_LIST).toList();
        QList<QVariant>::iterator begin, end;
        for (begin = argList.begin(), end = argList.end(); begin != end; begin++) {
            QString arg = (*begin).toString();
            if (arg.contains(' ')) {
                QStringList splittedArg = arg.split(' ');
                args << splittedArg;
            } else {
                args << arg;
            }
        }
    } else {
        if (filename.isEmpty()) {
            args << "tv://";
        } else {
            args << filename;
        }
    }

    previewProcess->setEnvironment(env);
    previewProcess->start(appBinaryName, args);
}

void MainWindow::configurePreview() {
    PreviewSettingsDialog dialog;
    int res = dialog.exec();
    if (res == QDialog::Accepted) {
        dialog.saveSettings();
    }
}

void MainWindow::previewProcError(QProcess::ProcessError er) {
    switch (er) {
    case QProcess::FailedToStart:
        QMessageBox::critical(NULL, "v4l2ucp", "Failed to start preview process!");
        break;
    case QProcess::Crashed:
        QMessageBox::critical(NULL, "v4l2ucp", "Preview process crashed!");
        break;
    default:
        break;
    }
}

void MainWindow::previewFinished(int exitCode, QProcess::ExitStatus status) {
    switch (status) {
    case QProcess::CrashExit:
        break;
    case QProcess::NormalExit:
        if (exitCode != 0) {
            QMessageBox::critical(NULL, "v4l2ucp", "Preview process exited with code != 0");
        }
        break;
    default:
        break;
    }
}
