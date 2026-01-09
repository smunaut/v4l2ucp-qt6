/*  v4l2ucp - A universal control panel for all V4L2 devices
    Copyright (C) 2005 Scott J. Bertin (scottbertin@yahoo.com)
    Copyright (C) 2009 Vasily Khoruzhick (anarsoul@gmail.com)

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
#include <QGridLayout>
#include <QMainWindow>
#include <QMenu>
#include <QProcess>
#include <QTimer>

class MainWindow : public QMainWindow {
    Q_OBJECT

  public slots:
    static void fileOpen();
    void updateDisabled();
    void update1Sec();
    void update5Sec();
    void update10Sec();
    void update20Sec();
    void update30Sec();
    void timerShot();
    void about();
    void aboutQt();
    void startPreview();
    void configurePreview();
    void previewProcError(QProcess::ProcessError er);
    void previewFinished(int exitCode, QProcess::ExitStatus status);

  signals:
    void updateNow();

  public:
    static MainWindow *openFile(const char *fileName);
    ~MainWindow();

  private:
    QMenu *updateMenu, *resetMenu;
    int fd;
    QAction *resetAllId;
    QAction *updateActions[6];
    QTimer timer;
    QProcess *previewProcess;
    QString filename;

    MainWindow(QWidget *parent = 0, const char *name = 0);
    void add_control(struct v4l2_queryctrl &ctrl, int fd, QWidget *parent, QGridLayout *);
};
