// Resources for Gens/GS Win32

// Icons
#define IDI_GENS_APP		0x0001
#define IDI_GENS_MD		0x0002
#define IDI_SONIC_HEAD		0x0003
#define IDI_SONIC_WAITING	0x0004

// Bitmaps
#define IDB_GENS_LOGO_SMALL	0x0101
#define IDB_GENS_LOGO_BIG	0x0102

// Buttons
#define IDC_BTN_OK		0x1001
#define IDC_BTN_CANCEL		0x1002
#define IDC_BTN_APPLY		0x1003
#define IDC_BTN_SAVE		0x1004
#define IDC_BTN_ADD		0x1005
#define IDC_BTN_DELETE		0x1006
#define IDC_BTN_DEACTIVATEALL	0x1007

// Change Button. (LOWORD(wParam) & 0xFF) == file ID.
#define IDC_BTN_CHANGE		0x1100

// Trackbars
#define IDC_TRK_CA_CONTRAST	0x2001
#define IDC_TRK_CA_BRIGHTNESS	0x2002

// Zip Select Dialog
#define IDD_ZIPSELECT		0x3000
#define IDC_ZIPSELECT_LSTFILES	0x3001

// Controller Configuration Dialog
#define IDD_CONTROLLER_CONFIG	0x4000

// Controller Configuration Dialog - Teamplayer checkboxes
#define IDC_CONTROLLER_CONFIG_CHKTEAMPLAYER	0x4010
#define IDC_CONTROLLER_CONFIG_CHKTEAMPLAYER1	0x4011
#define IDC_CONTROLLER_CONFIG_CHKTEAMPLAYER2	0x4012

// Controller Configuration Dialog - Reconfigure. (LOWORD(wParam) & 0xFF) == controller ID.
#define IDD_CONTROLLER_CONFIG_RECONFIGURE	0x4100

// Manifest
#define APP_MANIFEST		1
