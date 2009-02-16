/* museeq - a Qt client to museekd
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 * Copyright 2008 little blue poney <lbponey@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "settingsdialog.h"
#include "museeq.h"
#include "codeccombo.h"
#include "images.h"
#include "mainwin.h"
#include "util.h"
#include "museekdriver.h"

#include <QMenu>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QTreeWidget>
#include <QSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QColorDialog>
#include <QColor>
#include <QFontDialog>
#include <QLayout>
#include <QFileDialog>
#include <QDir>
#include <QCloseEvent>
#include <QSettings>
#include <QMessageBox>

SettingsDialog::SettingsDialog( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent ), mSharesDirty(false)
{
	// Layout Containing everything
	QVBoxLayout* vLayout= new QVBoxLayout( this);
	vLayout->setMargin(5);
	vLayout->setSpacing(5);

	// Tabs, without geometry set will resize to fill the dialog
	mTabHolder = new QTabWidget( this );

	// Add tabs to vLayout
	vLayout->addWidget(mTabHolder);

	QHBoxLayout* buttonsLayout= new QHBoxLayout;

	// Ok, Save, Cancel buttons
	QSpacerItem* spacer5 = new QSpacerItem( 200, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonsLayout->addItem( spacer5 );

	mOK = new QPushButton( this );
	mOK->setIcon(IMG("ok"));
	buttonsLayout->addWidget(mOK);

	mSave = new QPushButton( this );
	mSave->setIcon(IMG("save"));
	buttonsLayout->addWidget(mSave);

	mCancel = new QPushButton( this);
	mCancel->setIcon(IMG("cancel"));
	mCancel->setDefault( TRUE );
	buttonsLayout->addWidget(mCancel);

	// Add buttons to vLayout
	vLayout->addLayout(buttonsLayout);

	mTabHolder->setTabPosition( QTabWidget::North );

	// Create the Museeq tab
	mMuseeqTabs = new QTabWidget( this);
	mTabHolder->addTab( mMuseeqTabs, "" );
	// Create the Museekd tab
	mMuseekdTabs = new QTabWidget( mTabHolder );
	mTabHolder->addTab( mMuseekdTabs, "" );

    // Populate museekd tab
	// SERVER TAB
	mServerTab = new QWidget( mMuseekdTabs);
	mMuseekdTabs->addTab( mServerTab, "" );
	QVBoxLayout * ServerLayout = new QVBoxLayout( mServerTab);

	QGroupBox * groupBox = new QGroupBox(tr("Host"), mServerTab);
	ServerLayout->addWidget(groupBox);
	ServerGrid = new QGridLayout(groupBox);

	// Server Host
	serverHostLabel = new QLabel( mServerTab);
	ServerGrid->addWidget( serverHostLabel, 0, 0 );

	SServerHost = new QLineEdit( mServerTab);
	SServerHost->setMaxLength( 50 );
	ServerGrid->addWidget( SServerHost, 0, 1, 1, 1 );

	// Server Port
	serverPortLabel = new QLabel( mServerTab );
	serverPortLabel->setMargin( 0 );

	ServerGrid->addWidget( serverPortLabel, 1, 0 );
	SServerPort = new QSpinBox( mServerTab);
	SServerPort->setMaximum( 65535 );
	SServerPort->setValue( 0 );

	ServerGrid->addWidget( SServerPort, 1, 1, 1, 1 );
	// Server Username
	usernamelabel = new QLabel( mServerTab );
	ServerGrid->addWidget( usernamelabel, 2, 0 );

	SSoulseekUsername = new QLineEdit( mServerTab);
	ServerGrid->addWidget( SSoulseekUsername, 2, 1, 1, 1 );
	// Server Password
	passwordLabel = new QLabel( mServerTab );
	ServerGrid->addWidget( passwordLabel, 3, 0 );

	SSoulseekPassword = new QLineEdit( mServerTab );
	SSoulseekPassword->setEchoMode(QLineEdit::Password);
	ServerGrid->addWidget( SSoulseekPassword, 3, 1, 1, 1 );

	// Connect / Disconnect
	QHBoxLayout * SideLayout = new QHBoxLayout;
	ServerLayout->addLayout(SideLayout);
	QGroupBox * groupBox2 = new QGroupBox(tr("Server status"), mServerTab);
	SideLayout->addWidget(groupBox2);
	QVBoxLayout * DaemonCLayout = new QVBoxLayout;
	groupBox2->setLayout(DaemonCLayout);
	SConnect = new QPushButton( mServerTab);
	SConnect->setIcon(IMG("connect"));
	DaemonCLayout->addWidget( SConnect );

	SDisconnect = new QPushButton( mServerTab);
	SDisconnect->setIcon(IMG("disconnect"));

	DaemonCLayout->addWidget( SDisconnect );
	// Filesystem encoding
	QGridLayout * ServerGrid2 = new QGridLayout;
	SideLayout->addLayout(ServerGrid2);
	fEncodingLabel = new QLabel( mServerTab );
	ServerGrid2->addWidget( fEncodingLabel, 6, 0 );
	SFileSystemEncoding =  new CodecCombo("encoding", "filesystem", mServerTab, "encoding");
	ServerGrid2->addWidget( SFileSystemEncoding, 6, 1 , 1, 1 );
	// Network Encoding
	nEncodingLabel = new QLabel( mServerTab );
	ServerGrid2->addWidget( nEncodingLabel, 7, 0 );
	SNetworkEncoding =  new CodecCombo("encoding", "network", mServerTab, "encoding");
	ServerGrid2->addWidget( SNetworkEncoding, 7, 1, 1, 1 );

	spacerServer = new QSpacerItem( 20, 20, QSizePolicy::Preferred, QSizePolicy::Expanding );
	ServerLayout->addItem(spacerServer);



	// SHARES TAB
	mSharesTab = new QWidget( mMuseekdTabs);
	mMuseekdTabs->addTab( mSharesTab, "" );
	SharesGrid = new QGridLayout( mSharesTab);

	downloadLabel = new QLabel( mSharesTab);
	downloadLabel->setTextFormat(Qt::RichText);
	SharesGrid->addWidget( downloadLabel, 4, 0 );

	SDownDir = new QLineEdit( mSharesTab);
	SharesGrid->addWidget( SDownDir, 4, 1);

	SDownloadButton = new QPushButton( mSharesTab );
	SDownloadButton->setIcon(IMG("open"));
	SharesGrid->addWidget( SDownloadButton, 4, 2);

	incompleteLabel = new QLabel( mSharesTab);
	incompleteLabel->setTextFormat(Qt::RichText);
	SharesGrid->addWidget( incompleteLabel, 5, 0);

	SIncompleteDir = new QLineEdit( mSharesTab);
	SharesGrid->addWidget( SIncompleteDir, 5, 1);

	SIncompleteButton = new QPushButton( mSharesTab);
	SIncompleteButton->setIcon(IMG("open"));
	SharesGrid->addWidget( SIncompleteButton, 5, 2);


	// Shares List
	ListNormalShares = new QTreeWidget(mSharesTab);
	ListNormalShares->setRootIsDecorated(false);
	QStringList NormalSharesHeaders;
	NormalSharesHeaders <<  tr("Directories");
	ListNormalShares->setHeaderLabels(NormalSharesHeaders);
	SharesGrid->addWidget( ListNormalShares, 6,  0,  1, 2);
	// Shares List Buttons
	QVBoxLayout* sharesListButtons = new QVBoxLayout;
	SharesGrid->addLayout(sharesListButtons, 6, 2);

	NSharesRefresh = new QPushButton( mSharesTab );
	NSharesRefresh->setIcon( IMG("reload"));
	sharesListButtons->addWidget( NSharesRefresh);
	NSharesUpdate = new QPushButton( mSharesTab );
	NSharesUpdate->setIcon(IMG("redo"));
	sharesListButtons->addWidget( NSharesUpdate);
	NSharesRescan = new QPushButton( mSharesTab);
	NSharesRescan->setIcon(IMG("rescan"));
	sharesListButtons->addWidget( NSharesRescan);
	NSharesAdd = new QPushButton( mSharesTab);
	NSharesAdd->setIcon(IMG("add"));
	sharesListButtons->addWidget( NSharesAdd);
	NSharesRemove = new QPushButton( mSharesTab);
	NSharesRemove->setIcon(IMG("remove"));
	sharesListButtons->addWidget( NSharesRemove);

	// Buddy Shares List

	SBuddiesShares = new QCheckBox( mSharesTab);
	SharesGrid->addWidget( SBuddiesShares, 7, 0, 1, 2 );

	ListBuddyShares = new QTreeWidget(mSharesTab);
	ListBuddyShares->setRootIsDecorated(false);
	QStringList BuddySharesHeaders;
	BuddySharesHeaders <<  tr("Directories");
	ListBuddyShares->setHeaderLabels(BuddySharesHeaders);
	SharesGrid->addWidget( ListBuddyShares, 8,  0, 1, 2);
	// Buddy Shares List Buttons
	QVBoxLayout* sharesBuddyListButtons = new QVBoxLayout;
	SharesGrid->addLayout(sharesBuddyListButtons, 8, 2);
	SharesGrid->setRowStretch(6, 2);
	SharesGrid->setRowStretch(8, 2);
	BSharesRefresh = new QPushButton( mSharesTab);
	BSharesRefresh->setIcon( IMG("reload"));
	sharesBuddyListButtons->addWidget( BSharesRefresh);
	BSharesUpdate = new QPushButton( mSharesTab );
	BSharesUpdate->setIcon(IMG("redo"));
	sharesBuddyListButtons->addWidget( BSharesUpdate);
	BSharesRescan = new QPushButton( mSharesTab);
	BSharesRescan->setIcon(IMG("rescan"));
	sharesBuddyListButtons->addWidget( BSharesRescan);
	BSharesAdd = new QPushButton( mSharesTab);
	BSharesAdd->setIcon(IMG("add"));
	sharesBuddyListButtons->addWidget( BSharesAdd);
	BSharesRemove = new QPushButton( mSharesTab);
	BSharesRemove->setIcon(IMG("remove"));
	sharesBuddyListButtons->addWidget( BSharesRemove);



	// Connections Tab
	mConnectionsTab = new QWidget( mMuseekdTabs );
	mMuseekdTabs->addTab( mConnectionsTab, "" );
	ConnectionsGrid = new QGridLayout( mConnectionsTab);
	QGroupBox * connectionsBox = new QGroupBox(tr("Peer Connections"), mConnectionsTab);
	ConnectionsGrid->addWidget(connectionsBox, 0, 0, 1, 4);
	QHBoxLayout * cboxLayout = new QHBoxLayout(connectionsBox);
	SActive = new QRadioButton( mConnectionsTab );
	SPassive = new QRadioButton( SActive);

	cboxLayout->addWidget(SActive);
	cboxLayout->addWidget(SPassive);

	listenPortsLabel = new QLabel( mConnectionsTab);
	listenPortsLabel->setTextFormat(Qt::RichText);
	ConnectionsGrid->addWidget( listenPortsLabel, 1, 0, 1, 4);
	listenPortsStartLabel = new QLabel( mConnectionsTab);
	ConnectionsGrid->addWidget( listenPortsStartLabel, 2, 0, Qt::AlignCenter);

	CPortStart = new QSpinBox( mConnectionsTab );
	CPortStart->setMaximum( 65535 );
	CPortStart->setValue( 0 );
	ConnectionsGrid->addWidget( CPortStart, 2, 1, Qt::AlignCenter);

	listenPortsEndLabel = new QLabel( mConnectionsTab );
	ConnectionsGrid->addWidget( listenPortsEndLabel, 2, 2, Qt::AlignCenter);

	CPortEnd = new QSpinBox( mConnectionsTab);
	CPortEnd->setMaximum( 65535 );
	CPortEnd->setValue( 0 );

	ConnectionsGrid->addWidget( CPortEnd, 2, 3, Qt::AlignCenter);

	ConnectionsGrid->setRowStretch(3, 10);


	// USERS Options Tab
	mUsersTab = new QWidget( mMuseekdTabs);
	mMuseekdTabs->addTab( mUsersTab, "" );
	UsersGrid = new QGridLayout( mUsersTab);


	SBuddiesPrivileged = new QCheckBox( mUsersTab);
	UsersGrid->addWidget( SBuddiesPrivileged, 0, 0);

	SShareBuddiesOnly = new QCheckBox( mUsersTab );
	UsersGrid->addWidget( SShareBuddiesOnly, 1, 0);

	STrustedUsers = new QCheckBox( mUsersTab);
	UsersGrid->addWidget( STrustedUsers, 2, 0);

	SUserWarnings = new QCheckBox( mUsersTab );
	UsersGrid->addWidget( SUserWarnings, 3, 0);

	UsersGrid->setRowStretch(4, 10);


    // Populate museeq tab
    populateDConnectionTab();

	mAppearanceTab = new QWidget( mMuseeqTabs);
	mMuseeqTabs->addTab( mAppearanceTab, "" );
	AppearanceGrid = new QGridLayout( mAppearanceTab);

	mToggleTrayicon = new QCheckBox(tr("Enable &Trayicon"), mAppearanceTab);
	AppearanceGrid->addWidget( mToggleTrayicon, 0, 0, 1, 2  );

	mToggleLog = new QCheckBox(tr("Show &Log"), mAppearanceTab);
	AppearanceGrid->addWidget( mToggleLog, 1, 0, 1, 2  );

	SOnlineAlerts = new QCheckBox( mAppearanceTab );
	AppearanceGrid->addWidget( SOnlineAlerts, 2, 0, 1, 2  );

	SIPLog = new QCheckBox( mAppearanceTab);
	AppearanceGrid->addWidget( SIPLog, 3, 0, 1, 2  );

	mToggleTimestamps = new QCheckBox(tr("Show T&imestamps"), mAppearanceTab);
	AppearanceGrid->addWidget( mToggleTimestamps, 4, 0, 1, 2  );

	IconsAlignment = new QCheckBox( mAppearanceTab);
	AppearanceGrid->addWidget( IconsAlignment, 5, 0, 1, 2  );

    mToggleTickers = new QCheckBox(tr("Show &Tickers"), mAppearanceTab);
	AppearanceGrid->addWidget( mToggleTickers, 6, 0, 1, 2  );

	TickerLengthLabel = new QLabel(mAppearanceTab);
	AppearanceGrid->addWidget( TickerLengthLabel, 7, 0);

	TickerLength = new QSpinBox( mAppearanceTab);
	TickerLength->setMaximum( 500 );
	TickerLength->setMinimum( 20 );
	TickerLength->setValue( 20 );
	AppearanceGrid->addWidget( TickerLength, 7, 1 );
	AppearanceGrid->setColumnStretch(0, 8);
	AppearanceGrid->setRowStretch(9, 11);

	mIconTheme = new QPushButton(tr("Pick &Icon Theme... (Requires Restart)"), mAppearanceTab);
	connect(mIconTheme, SIGNAL(clicked()), parent, SLOT(changeTheme()));
	AppearanceGrid->addWidget( mIconTheme, 8, 0 );

	// Logging
	mLoggingTab = new QWidget( mMuseeqTabs);
	mMuseeqTabs->addTab( mLoggingTab, "" );
	LoggingGrid = new QGridLayout( mLoggingTab);

	LoggingPrivate = new QCheckBox( mLoggingTab );
	LoggingGrid->addWidget( LoggingPrivate, 0, 0);


	LoggingPrivateDir = new QLineEdit( mLoggingTab);
	LoggingGrid->addWidget( LoggingPrivateDir, 1, 0);

	LoggingPrivateButton = new QPushButton( mLoggingTab);
	LoggingPrivateButton->setIcon(IMG("open"));
	LoggingGrid->addWidget( LoggingPrivateButton, 1, 1);

	LoggingRooms = new QCheckBox( mLoggingTab);
	LoggingGrid->addWidget( LoggingRooms, 2, 0);

	LoggingRoomDir = new QLineEdit( mLoggingTab);
	LoggingGrid->addWidget( LoggingRoomDir, 3, 0);

	LoggingRoomButton = new QPushButton( mLoggingTab);
	LoggingRoomButton->setIcon(IMG("open"));
	LoggingGrid->addWidget( LoggingRoomButton, 3, 1);
	LoggingGrid->setRowStretch(4, 10);

	// Userinfo
	mUserInfoTab = new QWidget( mMuseekdTabs );
	mMuseekdTabs->addTab( mUserInfoTab, "" );
	UserInfoGrid = new QGridLayout( mUserInfoTab);

	mInfoText = new QTextEdit( mUserInfoTab );

	UserInfoGrid->addWidget( mInfoText, 0, 0, 1, 2 );

	mClear = new QRadioButton( mUserInfoTab);

	UserInfoGrid->addWidget( mClear, 3, 0 );

	mDontTouch = new QRadioButton( mClear);
	mDontTouch->setChecked( TRUE );

	UserInfoGrid->addWidget( mDontTouch, 1, 0 );

	mImage = new QLineEdit( mUserInfoTab );

	UserInfoGrid->addWidget( mImage, 2, 1 );

	mUpload = new QRadioButton( mClear );

	UserInfoGrid->addWidget( mUpload, 2, 0 );

	mBrowse = new QPushButton( mClear );
	mBrowse->setIcon(IMG("open"));
	UserInfoGrid->addWidget( mBrowse, 2, 2 );
	connect( mBrowse, SIGNAL( clicked() ), this, SLOT( UserImageBrowse_clicked() ) );

	// Protocol Handlers Tab
	mProtocolTab = new QWidget( mMuseeqTabs);
	mMuseeqTabs->addTab( mProtocolTab, "" );

	ProtocolGrid = new QGridLayout( mProtocolTab);

	mProtocols = new QTreeWidget( mProtocolTab);

	QStringList ProtocolsHeaders;
	ProtocolsHeaders <<  tr("Protocol") << tr("Handler");
	mProtocols->setHeaderLabels(ProtocolsHeaders);

	mProtocols->sortItems(0, Qt::AscendingOrder);
	mProtocols->setRootIsDecorated(false);
	mProtocols->setEditTriggers(QAbstractItemView::DoubleClicked);
	mProtocols->setAllColumnsShowFocus( TRUE );

	ProtocolGrid->addWidget( mProtocols, 0, 0, 1, 4 );

	mNewHandler = new QPushButton( mProtocolTab);
	mNewHandler->setIcon(IMG("new"));
	ProtocolGrid->addWidget( mNewHandler, 2, 2 );
	protocolSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	ProtocolGrid->addItem( protocolSpacer, 2, 0 );
	mModifyHandler = new QPushButton( mProtocolTab);
	mModifyHandler->setIcon(IMG("comments"));
	ProtocolGrid->addWidget( mModifyHandler, 2, 1 );
	// Colors And Fonts
	mColorsAndFontsTab = new QWidget( mMuseeqTabs );
	mMuseeqTabs->addTab( mColorsAndFontsTab, "" );
	ColorsGrid = new QGridLayout( mColorsAndFontsTab);
	MeColorLabel = new QLabel( mColorsAndFontsTab);
	ColorsGrid->addWidget( MeColorLabel, 0, 0 );

	SMeText = new QLineEdit( mColorsAndFontsTab );
	ColorsGrid->addWidget( SMeText, 0, 1 );

	MeColorButton = new QPushButton( mColorsAndFontsTab);
	MeColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( MeColorButton, 0, 2 );

	RemoteColorLabel = new QLabel( mColorsAndFontsTab );
	ColorsGrid->addWidget( RemoteColorLabel, 1, 0 );

	SRemoteText = new QLineEdit( mColorsAndFontsTab );
	ColorsGrid->addWidget( SRemoteText, 1, 1 );

	RemoteColorButton = new QPushButton( mColorsAndFontsTab);
	RemoteColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( RemoteColorButton, 1, 2 );

	LocalTextLabel = new QLabel( mColorsAndFontsTab );
	ColorsGrid->addWidget( LocalTextLabel, 2, 0 );

	SNicknameText = new QLineEdit( mColorsAndFontsTab );
	ColorsGrid->addWidget( SNicknameText, 2, 1 );

	NicknameColorButton = new QPushButton( mColorsAndFontsTab );
	NicknameColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( NicknameColorButton, 2, 2 );

	BuddiedColorLabel = new QLabel( mColorsAndFontsTab );
	ColorsGrid->addWidget( BuddiedColorLabel, 3, 0 );

	SBuddiedText = new QLineEdit( mColorsAndFontsTab);
	ColorsGrid->addWidget( SBuddiedText, 3, 1 );

	BuddiedColorButton = new QPushButton( mColorsAndFontsTab );
	BuddiedColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( BuddiedColorButton, 3, 2 );

	BannedColorLabel = new QLabel( mColorsAndFontsTab );
	ColorsGrid->addWidget( BannedColorLabel, 4, 0 );

	SBannedText = new QLineEdit( mColorsAndFontsTab );
	ColorsGrid->addWidget( SBannedText, 4, 1 );

	BannedColorButton = new QPushButton( mColorsAndFontsTab );
	BannedColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( BannedColorButton, 4, 2 );

	TrustColorLabel = new QLabel( mColorsAndFontsTab );
	ColorsGrid->addWidget( TrustColorLabel, 5, 0 );

	STrustedText = new QLineEdit( mColorsAndFontsTab);
	ColorsGrid->addWidget( STrustedText, 5, 1 );

	TrustColorButton = new QPushButton( mColorsAndFontsTab);
	TrustColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( TrustColorButton, 5, 2 );

	TimeColorLabel = new QLabel( mColorsAndFontsTab);
	TimeColorLabel->setTextFormat(Qt::RichText);
	ColorsGrid->addWidget( TimeColorLabel, 6, 0 );

	STimeText = new QLineEdit( mColorsAndFontsTab);
	ColorsGrid->addWidget( STimeText, 6, 1 );

	TimeColorButton = new QPushButton( mColorsAndFontsTab );
	TimeColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( TimeColorButton, 6, 2 );

	TimeFontLabel = new QLabel( mColorsAndFontsTab);
	TimeFontLabel->setTextFormat(Qt::RichText);
	ColorsGrid->addWidget( TimeFontLabel, 7, 0 );

	STimeFont = new QLineEdit( mColorsAndFontsTab );
	ColorsGrid->addWidget( STimeFont, 7, 1 );

	TimeFontButton = new QPushButton( mColorsAndFontsTab);
	TimeFontButton->setIcon(IMG("font"));
	ColorsGrid->addWidget( TimeFontButton, 7, 2 );

	MessageFontLabel = new QLabel( mColorsAndFontsTab );
	ColorsGrid->addWidget( MessageFontLabel, 8, 0 );

	SMessageFont = new QLineEdit( mColorsAndFontsTab );
	ColorsGrid->addWidget( SMessageFont, 8, 1 );

	MessageFontButton = new QPushButton( mColorsAndFontsTab );
	MessageFontButton->setIcon(IMG("font"));
	ColorsGrid->addWidget( MessageFontButton, 8, 2 );


	// Translate
	languageChange();
	resize( QSize(700, 600).expandedTo(minimumSizeHint()) );

	connect( MeColorButton, SIGNAL( clicked() ), this, SLOT( color_text_me() ) );
	connect( BannedColorButton, SIGNAL( clicked() ), this, SLOT( color_text_banned() ) );
	connect( BuddiedColorButton, SIGNAL( clicked() ), this, SLOT( color_text_buddied() ) );
	connect( MessageFontButton, SIGNAL( clicked() ), this, SLOT( font_text_message() ) );
	connect( NicknameColorButton, SIGNAL( clicked() ), this, SLOT( color_text_nickname() ) );
	connect( RemoteColorButton, SIGNAL( clicked() ), this, SLOT( color_text_remote() ) );
	connect( TimeColorButton, SIGNAL( clicked() ), this, SLOT( color_text_time() ) );
	connect( TrustColorButton, SIGNAL( clicked() ), this, SLOT( color_text_trusted() ) );
	connect( TimeFontButton, SIGNAL( clicked() ), this, SLOT( font_text_time() ) );

	// signals and slots connections
	connect( mOK, SIGNAL( clicked() ), this, SLOT( acceptSettings() ) );
	connect( mSave, SIGNAL( clicked() ), this, SLOT( save() ) );
	connect( mCancel, SIGNAL( clicked() ), this, SLOT( rejectSettings() ) );
	connect( SConnect, SIGNAL( clicked() ), this, SLOT( SConnect_clicked() ) );
	connect( SDisconnect, SIGNAL( clicked() ), this, SLOT( SDisconnect_clicked() ) );
	connect( SDownloadButton, SIGNAL( clicked() ), this, SLOT( SDownload_clicked() ) );
	connect( SIncompleteButton, SIGNAL( clicked() ), this, SLOT( SIncomplete_clicked() ) );

	connect( SBuddiesShares, SIGNAL( toggled(bool) ), SLOT( SBuddiesSharesToggled(bool) ) );

	connect( NSharesRefresh, SIGNAL( clicked() ), this, SLOT( NormalSharesRefresh() ) );
	connect( NSharesUpdate, SIGNAL( clicked() ), this, SLOT( NormalSharesUpdate() ) );
	connect( NSharesRescan, SIGNAL( clicked() ), this, SLOT( NormalSharesRescan() ) );
	connect( NSharesAdd, SIGNAL( clicked() ), this, SLOT( NormalSharesAdd() ) );
	connect( NSharesRemove, SIGNAL( clicked() ), this, SLOT( NormalSharesRemove() ) );


	connect( BSharesRefresh, SIGNAL( clicked() ), this, SLOT( BuddySharesRefresh() ) );
	SBuddiesSharesToggled(false);
	connect( BSharesUpdate, SIGNAL( clicked() ), this, SLOT( BuddySharesUpdate() ) );
	connect( BSharesRescan, SIGNAL( clicked() ), this, SLOT( BuddySharesRescan() ) );
	connect( BSharesAdd, SIGNAL( clicked() ), this, SLOT( BuddySharesAdd() ) );
	connect( BSharesRemove, SIGNAL( clicked() ), this, SLOT( BuddySharesRemove() ) );

	connect( LoggingPrivateButton, SIGNAL( clicked() ), this, SLOT( PrivateDirSelect() ) );
	connect( LoggingRoomButton, SIGNAL( clicked() ), this, SLOT( RoomDirSelect() ) );

	// Protocol treewidget signals
	mProtocols->setContextMenuPolicy(Qt::CustomContextMenu);
	mProtocolsMenu = new QMenu(this);

	ActionDeleteHandler = new QAction(IMG("remove"),tr("Delete handler"), this);
	connect(ActionDeleteHandler, SIGNAL(triggered()), this, SLOT(mProtocols_itemDelete()));
	mProtocolsMenu->addAction(ActionDeleteHandler);

	connect(mProtocols, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotProtocolContextMenu(const QPoint&)));
	connect( mNewHandler, SIGNAL( clicked() ), this, SLOT( mNewHandler_clicked() ) );
	connect( mModifyHandler, SIGNAL( clicked() ), this, SLOT( mModifyHandler_clicked() ) );

	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));

    loadSettings();
}

void SettingsDialog::populateDConnectionTab() {
    mDConnectionTab = new QWidget( mMuseeqTabs);
	mMuseeqTabs->addTab( mDConnectionTab, "" );

	QVBoxLayout * vLayout = new QVBoxLayout( mDConnectionTab );

    QGroupBox * groupBox2 = new QGroupBox(tr( "Daemon management:" ), mDConnectionTab);
    QVBoxLayout * daemonLayout = new QVBoxLayout;
    groupBox2->setLayout(daemonLayout);

	mMusetupButton = new QPushButton( this );
	mMusetupButton->setText( tr( "C&onfigure the daemon (Musetup)" ) );

	mStartDaemonButton = new QPushButton( this);
	mStartDaemonButton->setText( tr( "&Start Daemon" ) );

	mStopDaemonButton = new QPushButton( this );
	mStopDaemonButton->setText( tr( "S&top Daemon" ) );

	QHBoxLayout* hLayout4 = new QHBoxLayout;
	daemonLayout->addLayout(hLayout4);
	hLayout4->setMargin(5);
	hLayout4->setSpacing(5);
    hLayout4->addStretch(1);
	hLayout4->addWidget( mMusetupButton );
	hLayout4->addWidget( mStartDaemonButton );
	hLayout4->addWidget( mStopDaemonButton );
    hLayout4->addStretch(1);

	vLayout->addWidget( groupBox2 );


    vLayout->addSpacing(15);


    QGroupBox * groupBox = new QGroupBox(tr( "Daemon connection:" ), mDConnectionTab);
    QVBoxLayout * connectLayout = new QVBoxLayout;
    groupBox->setLayout(connectLayout);

	mHostLabel = new QLabel(mDConnectionTab);

    mDConnectType = new QComboBox(mDConnectionTab);
	mDConnectType->setEditable( false );
	mDConnectType->setSizePolicy (QSizePolicy::Expanding,QSizePolicy::Preferred);
	mDConnectType->addItem(tr( "TCP" ));
	mDConnectType->addItem(tr( "Unix socket" ));

	mDAddress = new QComboBox(mDConnectionTab);
	mDAddress->setEditable( true );
	mDAddress->setSizePolicy (QSizePolicy::Expanding,QSizePolicy::Preferred);

	mDClearButton = new QPushButton( mDConnectionTab);
	mDClearButton->setText( tr( "Clear" ) );

	QHBoxLayout* hLayout = new QHBoxLayout;
	connectLayout->addLayout(hLayout);
	hLayout->setMargin(5);
	hLayout->setSpacing(5);
	hLayout->addWidget( mHostLabel );
	hLayout->addWidget( mDConnectType );
	hLayout->addWidget( mDAddress );
	hLayout->addWidget( mDClearButton );
    hLayout->addStretch(1);


	mDPasswordLabel = new QLabel(mDConnectionTab);
	mDPasswordLabel->setText( tr( "Password:" ) );

	mDPassword = new QLineEdit;
	mDPassword->setEchoMode( QLineEdit::Password );

	mDSavePassword = new QCheckBox( this );
	mDSavePassword->setText( tr( "S&ave Password" ) );

	QHBoxLayout* hLayout2 = new QHBoxLayout;
	connectLayout->addLayout(hLayout2);
	hLayout2->setMargin(5);
	hLayout2->setSpacing(5);
	hLayout2->addWidget( mDPasswordLabel );
	hLayout2->addWidget( mDPassword );
	hLayout2->addWidget( mDSavePassword );
    hLayout2->addStretch(1);

	vLayout->addWidget( groupBox );

	mConnectToDaemonButton = new QPushButton( this);
	mConnectToDaemonButton->setText( tr( "&Connect to daemon" ) );
	mConnectToDaemonButton->setIcon(IMG("connect"));

	mDisconnectFromDaemonButton = new QPushButton( this );
	mDisconnectFromDaemonButton->setText( tr( "&Disconnect from daemon" ) );
	mDisconnectFromDaemonButton->setIcon(IMG("disconnect"));
	mDisconnectFromDaemonButton->setEnabled(false);

	QHBoxLayout* hLayout5 = new QHBoxLayout;
	connectLayout->addLayout(hLayout5);
	hLayout5->setMargin(5);
	hLayout5->setSpacing(5);
    hLayout5->addStretch(1);
	hLayout5->addWidget( mConnectToDaemonButton );
	hLayout5->addWidget( mDisconnectFromDaemonButton );
    hLayout5->addStretch(1);


    vLayout->addSpacing(15);


	mConfigFileLabel = new QLabel( this);
	mConfigFileLabel->setText( tr( "Museek Daemon Config:\n(leave empty for default)" ) );

	mMuseekConfigFile = new QLineEdit( this );

	mSelectConfigFileButton = new QPushButton( this );
	mSelectConfigFileButton->setText( tr( "Se&lect..." ) );
	mSelectConfigFileButton->setIcon(IMG("open"));

	QHBoxLayout* hLayout3 = new QHBoxLayout;
	vLayout->addLayout(hLayout3);
	hLayout3->setMargin(5);
	hLayout3->setSpacing(5);
	hLayout3->addWidget( mConfigFileLabel );
	hLayout3->addWidget( mMuseekConfigFile );
	hLayout3->addWidget( mSelectConfigFileButton );


    vLayout->addSpacing(15);


	mAutoStartDaemon = new QCheckBox( this);
	mAutoStartDaemon->setText( tr( "Auto-sta&rt museek daemon" ) );
	vLayout->addWidget(mAutoStartDaemon);

	mDAutoConnect = new QCheckBox(this);
	mDAutoConnect->setText( tr( "A&uto-connect to daemon" ) );
	vLayout->addWidget(mDAutoConnect);

	mShowExitDialog = new QCheckBox( this );
	mShowExitDialog->setText( tr( "Don't show e&xit dialog" ) );
	vLayout->addWidget(mShowExitDialog);

	mShutDownDaemonOnExit = new QCheckBox( this );
	mShutDownDaemonOnExit->setText( tr( "S&hutdown daemon on exit" ) );
	vLayout->addWidget(mShutDownDaemonOnExit);

    vLayout->addStretch(1);


	connect( mStartDaemonButton, SIGNAL( clicked() ), this, SLOT( startDaemon() ) );
	connect( mStopDaemonButton, SIGNAL( clicked() ), this, SLOT( stopDaemon() ) );
	connect( mConnectToDaemonButton, SIGNAL(clicked()), parent(), SLOT(connectToMuseek()));
	connect( mDClearButton, SIGNAL( clicked() ), this, SLOT( clearSockets() ) );
	connect( mMusetupButton, SIGNAL( clicked() ), this, SLOT( launchMusetup() ) );
	connect( mDSavePassword, SIGNAL(toggled(bool)), SLOT(toggleSavePassword(bool)) );
	connect( mSelectConfigFileButton, SIGNAL( clicked() ), this, SLOT( selectConfig() ) );
	connect( mShowExitDialog, SIGNAL(toggled(bool)), mShutDownDaemonOnExit, SLOT(setEnabled(bool)) );
	connect( museeq->driver(), SIGNAL(loggedIn(bool, const QString&)), this, SLOT(loggedIn(bool, const QString&)));
	connect( museeq, SIGNAL(disconnected()), SLOT(slotDisconnected()));
	connect( museeq->driver(), SIGNAL(error(QAbstractSocket::SocketError)), SLOT(slotError(QAbstractSocket::SocketError)));
}

void SettingsDialog::loadSettings() {
    // museekd settings
    SServerHost->setText(museeq->config("server", "host"));
    SSoulseekUsername->setText(museeq->config("server", "username"));
    SSoulseekPassword->setText(museeq->config("server", "password"));
    SServerPort->setValue(museeq->config("server", "port").toInt());
	SDownDir->setText(museeq->config("transfers", "download-dir"));
    SIncompleteDir->setText(museeq->config("transfers", "incomplete-dir"));
	mInfoText->setText(museeq->config("userinfo", "text"));
    CPortStart->setValue(museeq->config("clients.bind", "first").toInt());
    CPortEnd->setValue(museeq->config("clients.bind", "last").toInt());
    NormalSharesRefresh();
    BuddySharesRefresh();

    SBuddiesShares->setChecked(museeq->config("transfers", "have_buddy_shares") == "true");
    SShareBuddiesOnly->setChecked(museeq->config("transfers", "only_buddies") == "true");
    SBuddiesPrivileged->setChecked(museeq->config("transfers", "privilege_buddies") == "true");
    STrustedUsers->setChecked(museeq->config("transfers", "trusting_uploads") == "true");
    SUserWarnings->setChecked(museeq->config("transfers", "user_warnings") == "true");
    SActive->setChecked(museeq->config("clients", "connectmode") == "active");

    // museeq settings
    mMuseekConfigFile->setText(museeq->settings()->value("MuseekConfigFile").toString());
	mAutoStartDaemon->setChecked(museeq->settings()->value("LaunchMuseekDaemon").toBool());
	mDAutoConnect->setChecked(museeq->settings()->value("AutoConnect").toBool());
    mShowExitDialog->setChecked(!museeq->settings()->value("ShowExitDialog", true).toBool());
	mShutDownDaemonOnExit->setChecked(museeq->settings()->value("ShutDownDaemonOnExit").toBool());
	mShutDownDaemonOnExit->setEnabled(mShowExitDialog->isChecked());

 	mToggleTickers->setChecked(museeq->settings()->value("showTickers", true).toBool());
 	mToggleTimestamps->setChecked(museeq->settings()->value("showTimestamps", true).toBool());
 	museeq->mShowTimestamps = museeq->settings()->value("showTimestamps", true).toBool();
 	mToggleLog->setChecked(museeq->settings()->value("showStatusLog", false).toBool());

	IconsAlignment->setChecked(museeq->settings()->value("VerticalIconBox").toBool());
	SMessageFont->setText(museeq->mFontMessage);
	STimeFont->setText(museeq->mFontTime);
	STimeText->setText(museeq->mColorTime);
	SRemoteText->setText(museeq->mColorRemote);
	SMeText->setText(museeq->mColorMe);
	SNicknameText->setText(museeq->mColorNickname);
	SBuddiedText->setText(museeq->mColorBuddied);
	SBannedText->setText(museeq->mColorBanned);
	STrustedText->setText(museeq->mColorTrusted);
	LoggingRoomDir->setText(museeq->mRoomLogDir);
	LoggingPrivateDir->setText(museeq->mPrivateLogDir);
	LoggingPrivate->setChecked(museeq->mLogPrivate);
	LoggingRooms->setChecked(museeq->mLogRooms);
	SOnlineAlerts->setChecked(museeq->mOnlineAlert);
	SIPLog->setChecked(museeq->mIPLog);
	TickerLength->setValue(museeq->mTickerLength);

 	if (museeq->settings()->value("SavePassword").toBool())
	{
		mDSavePassword->setChecked(true);
		QString password = museeq->settings()->value("Password").toString();
		if ( !password.isEmpty())
			mDPassword->setText(password);
	}
	else {
		mDSavePassword->setChecked(false);
		mDPassword->setEnabled(false);
	}


    mDAddress->clear();
	museeq->settings()->beginGroup("Servers");
	QStringList s_keys = museeq->settings()->childKeys();
	museeq->settings()->endGroup();
	QString cServer;
	if(! s_keys.isEmpty()) {
		for(QStringList::Iterator it = s_keys.begin(); it != s_keys.end(); ++it)
		{
			cServer = museeq->settings()->value("Servers/" + (*it)).toString();
			mDAddress->addItem(cServer);
		}
		mDPassword->setFocus();
	} else {
		cServer = "localhost:2240";
		mDAddress->addItem(cServer);
		mDAddress->setFocus();
#ifdef HAVE_SYS_UN_H
# ifdef HAVE_PWD_H
		struct passwd *pw = getpwuid(getuid());
		if(pw)
			mDAddress->addItem(QString("/tmp/museekd.") + QString(pw->pw_name));
# endif
#endif
	}

	mDAddress->setCurrentIndex(mDAddress->count() - 1);
}

void SettingsDialog::acceptSettings() {
    save();
    hide();
}

void SettingsDialog::rejectSettings() {
    loadSettings();
    hide();
}

void SettingsDialog::slotConfigChanged(const QString& domain, const QString& key, const QString& value) {
	if(domain == "server" && key == "host") {
		SServerHost->setText(value);
	} else if(domain == "server" && key == "username") {
		SSoulseekUsername->setText(value);
	} else if(domain == "server" && key == "password") {
		SSoulseekPassword->setText(value);
	} else if(domain == "server" && key == "port") {
		SServerPort->setValue(value.toInt());
	} else if(domain == "transfers" && key == "have_buddy_shares") {
		if  (value == "true")  { SBuddiesShares->setChecked(true); }
		else if (value == "false") { SBuddiesShares->setChecked(false); }
	} else if(domain == "shares" && key == "database") {
		NormalSharesRefresh();
		BuddySharesRefresh();
	} else if(domain == "transfers" && key == "only_buddies") {
		if  (value == "true")  { SShareBuddiesOnly->setChecked(true); }
		else if (value == "false") { SShareBuddiesOnly->setChecked(false); }
	} else if(domain == "transfers" && key == "privilege_buddies") {
		if (value == "true") SBuddiesPrivileged->setChecked(true);
		else if (value == "false") SBuddiesPrivileged->setChecked(false);
	} else if(domain == "transfers" && key == "trusting_uploads") {
		if (value == "true") STrustedUsers->setChecked(true);
		else if ( value == "false") STrustedUsers->setChecked(false);
	} else if(domain == "transfers" && key == "user_warnings") {
		if (value == "true") SUserWarnings->setChecked(true);
		else if ( value == "false") SUserWarnings->setChecked(false);
	} else if(domain == "transfers" && key == "download-dir") {
		SDownDir->setText(value);
	} else if(domain == "transfers" && key == "incomplete-dir") {
		SIncompleteDir->setText(value);

	} else if(domain == "clients" && key == "connectmode") {
		if (value == "active") SActive->setChecked(true);
		else if (value == "passive") SPassive->setChecked(true);

	} else if (domain == "clients.bind") {
		if (key =="first") {
			CPortStart->setValue(value.toInt());
		} else if (key == "last") {
			CPortEnd->setValue(value.toInt());
		}
	} else if(domain == "userinfo" && key == "text") {
		mInfoText->setText(value);

	}

}

void SettingsDialog::SBuddiesSharesToggled(bool on) {
	ListBuddyShares->setEnabled(on);
	EnableBuddyButtons(on);
    mSharesDirty = true;
}

void SettingsDialog::NormalSharesRefresh() {
	ListNormalShares->clear();
	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(readyReadStandardOutput()), this, SLOT(readNormal()) );
	connect( proc1, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(finishedListNormal( int, QProcess::ExitStatus)) );
	QStringList arguments ;
    QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    if (! museekConfig.isEmpty() ) {
        arguments.append("-c");
        arguments.append(museekConfig);
    }
	arguments.append("-l" );

	proc1->start( "muscan", arguments );
    mSharesDirty = true;
}

void SettingsDialog::BuddySharesRefresh() {
	ListBuddyShares->clear();
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(readyReadStandardOutput()), this, SLOT(readBuddy()) );
	connect( proc2, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(finishedListBuddy( int, QProcess::ExitStatus)) );
	QStringList arguments;
    QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    if (! museekConfig.isEmpty() ) {
        arguments.append("-c");
        arguments.append(museekConfig);
    }
	arguments.append("-l" );
	arguments.append("-b" );

	proc2->start( "muscan", arguments );
    mSharesDirty = true;
}

void SettingsDialog::BuddySharesAdd() {
	QFileDialog * fd = new QFileDialog(this, tr("Select a Directory to add to your Buddy Shares."), QDir::homePath());
	fd->setFileMode(QFileDialog::Directory);
	fd->setFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{

		EnableBuddyButtons(false);
		proc2 = new QProcess( this );
		connect( proc2, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedBuddy( int, QProcess::ExitStatus)) );

		QStringList arguments;
        QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
        if (! museekConfig.isEmpty() ) {
            arguments.append("-c");
            arguments.append(museekConfig);
        }
		arguments.append("-b" );
		arguments.append("-s" );
		arguments.append(fd->selectedFiles().at(0) );
		proc2->start( "muscan", arguments );

	}
	delete fd;
    mSharesDirty = true;
}

void SettingsDialog::PrivateDirSelect() {
	QString path = LoggingPrivateDir->text();
	if (path.isEmpty())
		path = QDir::homePath();
	QFileDialog * fd = new QFileDialog(this, tr("Select a Directory to write Private Chat log files."), path);
	fd->setFileMode(QFileDialog::Directory);
	fd->setViewMode(QFileDialog::Detail);
	fd->setFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{
		LoggingPrivateDir->setText(fd->selectedFiles().at(0));

	}
	delete fd;
}

void SettingsDialog::RoomDirSelect() {

	QString  path = LoggingRoomDir->text();
	if (path.isEmpty())
		path = QDir::homePath();
	QFileDialog * fd = new QFileDialog(this, tr("Select a Directory to write Chat Room log files."), path);
	fd->setFileMode(QFileDialog::Directory);
	fd->setViewMode(QFileDialog::Detail);

	fd->setFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{
		LoggingRoomDir->setText(fd->selectedFiles().at(0));

	}
	delete fd;
}

void SettingsDialog::BuddySharesRescan() {
	EnableBuddyButtons(false);
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedBuddy(int, QProcess::ExitStatus)) );

	QStringList arguments ;
    QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    if (! museekConfig.isEmpty() ) {
        arguments.append("-c");
        arguments.append(museekConfig);
    }
	arguments.append("-b");
	arguments.append("-r");

	proc2->start( "muscan", arguments );
    mSharesDirty = true;
}

void SettingsDialog::BuddySharesUpdate() {
	EnableBuddyButtons(false);
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedBuddy(int, QProcess::ExitStatus)) );

	QStringList arguments ;
    QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    if (! museekConfig.isEmpty() ) {
        arguments.append("-c");
        arguments.append(museekConfig);
    }
	arguments.append("-b" );

	proc2->start( "muscan", arguments );
    mSharesDirty = true;
}

void SettingsDialog::NormalSharesAdd() {
	QFileDialog * fd = new QFileDialog(this, tr("Select a Directory to add to your Normal Shares."), QDir::homePath());
	fd->setFileMode(QFileDialog::Directory);
	fd->setFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{
		EnableNormalButtons(false);
		proc1 = new QProcess( this );
		connect( proc1, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedNormal(int, QProcess::ExitStatus)) );

		QStringList arguments ;
        QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
        if (! museekConfig.isEmpty() ) {
            arguments.append("-c");
            arguments.append(museekConfig);
        }
		arguments.append("-s" );
		arguments.append(fd->selectedFiles().at(0) );
		proc1->start( "muscan", arguments );

	}
	delete fd;
    mSharesDirty = true;
}

void SettingsDialog::BuddySharesRemove() {
	QTreeWidgetItem* item = ListBuddyShares->currentItem();
	if (! item ||  item->text(0).isEmpty())
		return;
	QString directory (item->text(0));

	EnableBuddyButtons(false);
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedBuddy(int, QProcess::ExitStatus)) );

	QStringList arguments ;
    QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    if (! museekConfig.isEmpty() ) {
        arguments.append("-c");
        arguments.append(museekConfig);
    }
	arguments.append("-b" );
	arguments.append("-u" );
	arguments.append(directory );
	proc2->start( "muscan", arguments );

    mSharesDirty = true;
}

void SettingsDialog::NormalSharesRemove() {
	QTreeWidgetItem* item = ListNormalShares->currentItem();
	if (! item ||  item->text(0).isEmpty())
		return;

	QString directory (item->text(0));

	EnableNormalButtons(false);
	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedNormal(int, QProcess::ExitStatus)) );
	QStringList arguments ;
    QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    if (! museekConfig.isEmpty() ) {
        arguments.append("-c");
        arguments.append(museekConfig);
    }
	arguments.append("-u" );
	arguments.append(directory );
	proc1->start( "muscan", arguments );

    mSharesDirty = true;
}

void SettingsDialog::NormalSharesRescan() {
	EnableNormalButtons(false);

	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedNormal(int, QProcess::ExitStatus)) );
	QStringList arguments ;
    QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    if (! museekConfig.isEmpty() ) {
        arguments.append("-c");
        arguments.append(museekConfig);
    }
	arguments.append("-r" );

	proc1->start( "muscan", arguments );

    mSharesDirty = true;
}

void SettingsDialog::NormalSharesUpdate() {
	EnableNormalButtons(false);

	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedNormal(int, QProcess::ExitStatus)) );
	QStringList arguments ;
    QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    if (! museekConfig.isEmpty() ) {
        arguments.append("-c");
        arguments.append(museekConfig);
    }

	proc1->start( "muscan", arguments );

    mSharesDirty = true;
}

void SettingsDialog::EnableBuddyButtons(bool on) {
	BSharesRefresh->setEnabled(on);
	BSharesAdd->setEnabled(on);
	BSharesRemove->setEnabled(on);
	BSharesRescan->setEnabled(on);
	BSharesUpdate->setEnabled(on);
}

void SettingsDialog::EnableNormalButtons(bool on) {
	NSharesRefresh->setEnabled(on);
	NSharesAdd->setEnabled(on);
	NSharesRemove->setEnabled(on);
	NSharesRescan->setEnabled(on);
	NSharesUpdate->setEnabled(on);
}

void SettingsDialog::readNormal() {
	while (proc1->bytesAvailable()) {
		QString output = proc1->readLine();
		if (! output.isEmpty() ) {
			QTreeWidgetItem * item = new QTreeWidgetItem(ListNormalShares);
			item->setText(0, output.replace("\n", ""));
		}
	}
}

void SettingsDialog::readBuddy() {
	while (proc2->bytesAvailable()) {
		QString output = proc2->readLine();
		if (! output.isEmpty() ) {
			QTreeWidgetItem * item = new QTreeWidgetItem(ListBuddyShares);
			item->setText(0, output.replace("\n", ""));
		}
	}
}

void SettingsDialog::finishedListNormal( int exitCode, QProcess::ExitStatus exitStatus) {
	EnableNormalButtons(true);
}

void SettingsDialog::finishedListBuddy( int exitCode, QProcess::ExitStatus exitStatus) {
	EnableBuddyButtons(SBuddiesShares->isChecked());
}

void SettingsDialog::finishedNormal( int exitCode, QProcess::ExitStatus exitStatus) {
	EnableNormalButtons(true);
	museeq->mainwin()->appendToLogWindow(QString("Normal shares successfully reloaded."));
	NormalSharesRefresh();
}

void SettingsDialog::finishedBuddy( int exitCode, QProcess::ExitStatus exitStatus) {
	EnableBuddyButtons(SBuddiesShares->isChecked());
	museeq->mainwin()->appendToLogWindow(QString("Buddy shares successfully reloaded."));
	BuddySharesRefresh();
}

void SettingsDialog::SConnect_clicked()
{
    museeq->connectServer();
}

void SettingsDialog::SDisconnect_clicked()
{
    museeq->disconnectServer();
}

void SettingsDialog::save()
{
    museeq->saveSettings();
}

void SettingsDialog::SDownload_clicked()
{
    QFileDialog * fd = new QFileDialog(this, tr("Select a Directory to store your downloaded files."), QDir::homePath());
    fd->setFileMode(QFileDialog::Directory);
    fd->setFilter(tr("All files (*)"));
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
    {
        SDownDir->setText( fd->selectedFiles().at(0));
    }

    delete fd;
}

void SettingsDialog::SIncomplete_clicked()
{
    QFileDialog * fd = new QFileDialog(this, tr("Select a Directory to store your incomplete downloading files."), QDir::homePath());
    fd->setFileMode(QFileDialog::Directory);
    fd->setFilter(tr("All files (*)"));
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
    {
	SIncompleteDir->setText( fd->selectedFiles().at(0));
    }

    delete fd;
}

void SettingsDialog::UserImageBrowse_clicked()
{
	QFileDialog * fd = new QFileDialog(this, tr("Select an image for your User info"), QDir::homePath(), tr("Images (*.png *.gif *.jpg *.jpeg)"));
	fd->setFileMode(QFileDialog::ExistingFile);
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{
		mImage->setText(fd->selectedFiles().at(0));
		mUpload->setChecked(true);
	}

	delete fd;
}

void SettingsDialog::mNewHandler_clicked()
{
	QTreeWidgetItem* item = new QTreeWidgetItem(mProtocols);
	item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled );
	mProtocols->editItem(item, 0);
}

void SettingsDialog::mModifyHandler_clicked()
{
	QTreeWidgetItem* item = mProtocols->currentItem();
	if (item) {
		mProtocols->editItem(item, 1);
	}
}

void SettingsDialog::slotProtocolContextMenu(const QPoint& pos) {
	QTreeWidgetItem * item = mProtocols->itemAt(pos);
	if(! item)
		return;
	mProtocolsMenu->exec(mProtocols->mapToGlobal(pos));
}

void SettingsDialog::mProtocols_itemDelete() {
	QTreeWidgetItem * item = mProtocols->currentItem();
	if (! item)
		return;
	delete item;
}

void SettingsDialog::color_text_me()
{
   QColor c = QColorDialog::getColor( SMeText->text(), this );
    if ( c.isValid() )
	SMeText->setText(c.name());
}

void SettingsDialog::color_text_buddied()
{
   QColor c = QColorDialog::getColor( SBuddiedText->text(), this );
    if ( c.isValid() )
	SBuddiedText->setText(c.name());
}

void SettingsDialog::color_text_nickname()
{
   QColor c = QColorDialog::getColor( SNicknameText->text(), this );
    if ( c.isValid() )
	SNicknameText->setText(c.name());
}

void SettingsDialog::color_text_banned()
{
   QColor c = QColorDialog::getColor( SBannedText->text(), this );
    if ( c.isValid() )
	SBannedText->setText(c.name());
}

void SettingsDialog::color_text_remote()
{
   QColor c = QColorDialog::getColor( SRemoteText->text(), this );
    if ( c.isValid() )
	SRemoteText->setText(c.name());
}

void SettingsDialog::color_text_time()
{
   QColor c = QColorDialog::getColor( STimeText->text(), this );
    if ( c.isValid() )
	STimeText->setText(c.name());
}

void SettingsDialog::color_text_trusted()
{
   QColor c = QColorDialog::getColor( STrustedText->text(), this );
    if ( c.isValid() )
	STrustedText->setText(c.name());
}

void SettingsDialog::font_text_time()
{
    bool ok;
    QFont font = QFontDialog::getFont( &ok, STimeFont );
    if ( ok ) {
		QVariant s (font.pointSize());
 		QVariant w ( font.weight());
		QString c = ("font-family:"+ font.family() +";weight:"+w.toString()+";font-size:"+ s.toString()+"pt");
		STimeFont->setText(c);

    }
}

void SettingsDialog::font_text_message()
{
    bool ok;
    QFont font = QFontDialog::getFont( &ok, SMessageFont );
    if ( ok ) {
		QVariant s (font.pointSize());
 		QVariant w ( font.weight());
		QString c = ("font-family:"+ font.family() +";weight:"+w.toString()+";font-size:"+ s.toString()+"pt");
		SMessageFont->setText(c);

    }
}
/*
 *  Destroys the object and frees any allocated resources
 */
SettingsDialog::~SettingsDialog()
{
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SettingsDialog::languageChange()
{
	setWindowTitle( tr( "Museeq Settings" ) );
	mOK->setText( tr( "Ok" ) );
	mSave->setText( tr( "Save" ) );
	mCancel->setText( tr( "Cancel" ) );
	NSharesRefresh->setText( tr( "Refresh list" ) );
	NSharesUpdate->setText( tr( "Update" ) );
	NSharesRescan->setText( tr( "Rescan" ) );
	NSharesAdd->setText( tr( "Add" ) );
	NSharesRemove->setText( tr( "Remove" ) );

	BSharesRefresh->setText( tr( "Refresh list" ) );
	BSharesUpdate->setText( tr( "Update" ) );
	BSharesRescan->setText( tr( "Rescan" ) );
	BSharesAdd->setText( tr( "Add" ) );
	BSharesRemove->setText( tr( "Remove" ) );
	listenPortsLabel->setText( tr( "Listen port range (the first available port will be used when museekd starts up)" ) );
	listenPortsStartLabel->setText( tr( "First port:" ) );
	listenPortsEndLabel->setText( tr( "Last port:" ) );

	mClear->setText( tr( "Clear" ) );
	mDontTouch->setText( tr( "Don't touch" ) );
	mUpload->setText( tr( "Upload:" ) );
	mBrowse->setText( tr( "Select.." ) );


	LoggingPrivate->setText( tr( "Log Private Chats" ) );
	LoggingRooms->setText( tr( "Log Chat Rooms" ) );
	LoggingPrivateButton->setText( tr( "Select.." ) );
	LoggingRoomButton->setText( tr( "Select.." ) );

	fEncodingLabel->setText( tr( "Filesystem Encoding:" ) );
	nEncodingLabel->setText( tr( "Network Encoding:" ) );
	SConnect->setText( tr( "Connect" ) );
	SDisconnect->setText( tr( "Disconnect" ) );
	serverPortLabel->setText( tr( "Server Port:" ) );
	serverHostLabel->setText( tr( "Server Host:" ) );
	usernamelabel->setText( tr( "Soulseek Username:" ) );
	passwordLabel->setText( tr( "Soulseek Password:" ) );
	SSoulseekPassword->setInputMask( QString::null );

	mNewHandler->setText( tr( "New" ) );
	mModifyHandler->setText( tr( "Modify" ) );
	// Museekd Tabs
	mTabHolder->setTabText( mTabHolder->indexOf(mMuseekdTabs), tr( "Museek Daemon" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(mServerTab), tr( "Server" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(mSharesTab), tr( "Shares" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(mConnectionsTab), tr( "Connections" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(mUsersTab), tr( "User Options" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(mUserInfoTab), tr( "User Info" ) );
	// Museeq tabs
	mTabHolder->setTabText( mTabHolder->indexOf(mMuseeqTabs), tr( "Museeq" ) );
	mMuseeqTabs->setTabText( mMuseeqTabs->indexOf(mDConnectionTab), tr( "Daemon connection" ) );
	mMuseeqTabs->setTabText( mMuseeqTabs->indexOf(mAppearanceTab), tr("Appearance") );
	mMuseeqTabs->setTabText( mMuseeqTabs->indexOf(mColorsAndFontsTab), tr("Fonts and Colors") );
	mMuseeqTabs->setTabText( mMuseeqTabs->indexOf(mLoggingTab), tr( "Logging" ) );
	mMuseeqTabs->setTabText( mMuseeqTabs->indexOf(mProtocolTab), tr( "Protocol handlers" ) );
	// Fonts and Colors

	TimeFontLabel->setText( tr( "Time & Brackets Font" ) );
	TimeFontButton->setText( tr( "Pick Font" ) );
	TimeColorLabel->setText( tr( "Time & Brackets Text Color" ) );
	MeColorLabel->setText( tr( "/Me Text" ) );
	MeColorButton->setText( tr( "Pick Color" ) );
	NicknameColorButton->setText( tr( "Pick Color" ) );
	TimeColorButton->setText( tr( "Pick Color" ) );
	MessageFontLabel->setText( tr( "Message Font" ) );
	MessageFontButton->setText( tr( "Pick Font" ) );
	BuddiedColorLabel->setText( tr( "Buddied Users" ) );
	BuddiedColorButton->setText( tr( "Pick Color" ) );
	LocalTextLabel->setText( tr( "My Text" ) );
	TrustColorLabel->setText( tr( "Trusted Users" ) );
	TrustColorButton->setText( tr( "Pick Color" ) );
	BannedColorLabel->setText( tr( "Banned Users" ) );
	BannedColorButton->setText( tr( "Pick Color" ) );
	RemoteColorLabel->setText( tr( "Remote Text" ) );
	RemoteColorButton->setText( tr( "Pick Color" ) );



	// Connections and Ports
	SActive->setText( tr( "Active Connections" ) );
	SPassive->setText( tr( "Passive Connections" ) );
	SDownloadButton->setText( tr( "Select.." ) );
	downloadLabel->setText( tr( "Download Dir:" ) );
	SIncompleteButton->setText( tr( "Select.." ) );
	incompleteLabel->setText( tr( "Incomplete Dir:" ) );


	SBuddiesPrivileged->setText( tr( "Buddies are Privileged" ) );
	SOnlineAlerts->setText( tr( "Online Alerts in Log Window instead of popup" ) );
	SShareBuddiesOnly->setText( tr( "Share to Buddies Only" ) );
	STrustedUsers->setText( tr( "Trusted users can Send you Files" ) );
	SBuddiesShares->setText( tr( "Additional Shares for Buddies" ) );
	SUserWarnings->setText( tr( "Send automatic warnings to users via Private Chat" ) );
	SIPLog->setText( tr( "IP addresses in Log Window instead of popup" ) );
	TickerLengthLabel->setText( tr( "Maximum length of ticker messages:" ) );
	IconsAlignment->setText( tr( "Align Mode Icons Vertically" ) );

	mHostLabel->setText( tr( "Host / path:" ) );
}

void SettingsDialog::closeEvent(QCloseEvent * ev) {
    rejectSettings();
    ev->ignore();
}

// below be dragons
void SettingsDialog::startDaemon()
{
    museeq->startDaemon();
    mStartDaemonButton->setDisabled(true);
    mStopDaemonButton->setDisabled(false);
}

void SettingsDialog::stopDaemon()
{
    museeq->stopDaemon();
    mStartDaemonButton->setDisabled(false);
    mStopDaemonButton->setDisabled(true);
}

void SettingsDialog::showEvent( QShowEvent * event ) {
    bool locked = Util::getMuseekdLock();
    if (locked) {
        mStartDaemonButton->setDisabled(true);
        mStopDaemonButton->setDisabled(false);
    }
    else {
        mStartDaemonButton->setDisabled(false);
        mStopDaemonButton->setDisabled(true);
    }
}

void SettingsDialog::clearSockets() {
	mDAddress->clear();
}

void SettingsDialog::launchMusetup() {
    if (mTabHolder->isTabEnabled(mTabHolder->indexOf(mMuseekdTabs))) {
        mTabHolder->setCurrentWidget(mMuseekdTabs);
    }
    else {
        mSetupProc = new QProcess( this );
        connect( mSetupProc, SIGNAL(error( QProcess::ProcessError )), this, SLOT(musetupError( QProcess::ProcessError)) );
        QStringList arguments ;

        mSetupProc->start( "musetup-qt", arguments );
    }
}

void SettingsDialog::toggleSavePassword(bool on) {
    if (!on)
        mDPassword->clear();
    mDPassword->setEnabled(on);
}

void SettingsDialog::musetupError( QProcess::ProcessError error) {
	QMessageBox::warning(this, tr("Musetup error"), tr("Couldn't launch musetup-qt. Check that it is correctly installed. You can also launch musetup in a terminal using this command: 'musetup'."));
}

void SettingsDialog::loggedIn(bool success, const QString& msg) {
    mConnectToDaemonButton->setEnabled(!success);
    mDisconnectFromDaemonButton->setEnabled(success);
	if (!success && isVisible())
        QMessageBox::warning(this, tr("Connection error"), tr("Couldn't connect to the daemon. Please, check daemon connection settings (wrong password?)."));
    else if ( !success && QMessageBox::question(this, tr("Connection error"), tr("Couldn't connect to the daemon. Please, check daemon connection settings (wrong password?). Would you like to change museeq settings?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ==  QMessageBox::Yes)
        museeq->mainwin()->changeSettings();
}

void SettingsDialog::slotDisconnected() {
	mConnectToDaemonButton->setEnabled(true);
	mDisconnectFromDaemonButton->setEnabled(false);
}

void SettingsDialog::slotError(QAbstractSocket::SocketError e) {
	mConnectToDaemonButton->setEnabled(true);
	mDisconnectFromDaemonButton->setEnabled(false);
	if (isVisible())
        QMessageBox::warning(this, tr("Connection error"), tr("Couldn't connect or lost connection to the daemon. Please, check that the daemon is running and that it is listening at the given address."));
    else if ( QMessageBox::question(this, tr("Connection error"), tr("Couldn't connect or lost connection to the daemon. Please, check that the daemon is running and that it is listening at the given address. Would you like to change museeq settings?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ==  QMessageBox::Yes)
        museeq->mainwin()->changeSettings();
}

void SettingsDialog::selectConfig() {
    QDir dir = QDir::home();
    QFileDialog * fd = new QFileDialog(this, tr("Select a Museek Daemon Config File"), dir.path()+"/.museekd", "Museek Daemon Config (*.xml)");
    fd->setFileMode(QFileDialog::ExistingFile);
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
        mMuseekConfigFile->setText(fd->selectedFiles().at(0));

    delete fd;
}
