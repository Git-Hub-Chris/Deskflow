/*
 * synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2012 Symless Ltd.
 * Copyright (C) 2008 Volker Lanz (vl@fidra.de)
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "ui_SettingsDialogBase.h"

#include "CoreInterface.h"
#include "validators/ScreenNameValidator.h"

#include <QDialog>
#include <memory>

class MainWindow;
class AppConfig;

class SettingsDialog : public QDialog, public Ui::SettingsDialogBase {
  Q_OBJECT

public:
  SettingsDialog(QWidget *parent, AppConfig &config);
  static QString browseForSynergyc(
      QWidget *parent, const QString &programDir,
      const QString &coreClientName);
  static QString browseForSynergys(
      QWidget *parent, const QString &programDir,
      const QString &coreServerName);

protected:
  void accept() override;
  void reject() override;
  AppConfig &appConfig() { return m_appConfig; }

  /// @brief Load all settings.
  void loadFromConfig();

  /// @brief Enables or disables the TLS regenerate button.
  void updateTlsRegenerateButton();

  /// @brief Updates the key length value based on the loaded file.
  void updateKeyLengthOnFile(const QString &path);

  /// @brief Enables controls when they should be.
  void updateControlsEnabled();

  bool isClientMode() const;
  void updateTlsControls();
  void updateTlsControlsEnabled();

private:
  MainWindow *m_pMainWindow;
  AppConfig &m_appConfig;
  CoreInterface m_CoreInterface;
  std::unique_ptr<validators::ScreenNameValidator> m_screenNameValidator;

  /// @brief Stores settings scope at start of settings dialog
  /// This is neccessary to restore state if user changes
  /// the scope and doesn't save changes
  bool m_isSystemAtStart = false;

  QString m_nameError = "";

private slots:
  void on_m_pCheckBoxEnableCrypto_clicked(bool checked);
  void on_m_pCheckBoxLogToFile_stateChanged(int);
  void on_m_pButtonBrowseLog_clicked();

  /// @brief Handles the toggling of the system scoped radio button
  ///        As the user scope radio is connected this will fire for either
  ///        radio button
  void on_m_pRadioSystemScope_toggled(bool checked);

  /// @brief Handles the click event of the Cert Path browse button
  ///        displaying a file browser
  void on_m_pPushButtonBrowseCert_clicked();

  /// @brief Handles the TLS cert key length changed event
  void on_m_pComboBoxKeyLength_currentIndexChanged(int index);

  /// @brief handels the regenerate cert button event
  ///         This will regenerate the TLS certificate as long as the settings
  ///         haven't changed
  void on_m_pPushButtonRegenCert_clicked();

  void on_m_pScreenNameValidator_finished(const QString &error);
};
