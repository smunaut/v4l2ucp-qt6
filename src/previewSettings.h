/*  v4l2ucp - A universal control panel for all V4L2 devices
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

#include "ui_previewSettings.h"
#include <QDialog>

#define APP_ORG "v4l2ucp"
#define APP_NAME "v4l2ucp"
#define SETTINGS_APP_BINARY_NAME "preview/app_binary_name"
#define SETTINGS_ENV_LIST "preview/env_list"
#define SETTINGS_ARG_LIST "preview/arg_list"

class QListWidgetItem;

class PreviewSettingsDialog : public QDialog {
    Q_OBJECT

  public slots:
    void envItemDoubleClicked(QListWidgetItem *item);
    void argItemDoubleClicked(QListWidgetItem *item);
    void listCurItemChanged(QListWidgetItem *newItem, QListWidgetItem *oldItem);

    void addEnvItemClicked();
    void addArgItemClicked();
    void delEnvItemClicked();
    void delArgItemClicked();
    void defaultsClicked();

  public:
    PreviewSettingsDialog(QWidget *parent = NULL);
    ~PreviewSettingsDialog();
    void saveSettings();

  protected:
    Ui::previewSettingsDialog ui;

  private:
    void loadSettings();
    void connectSignals();
};
