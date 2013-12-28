/*
 * Freetronics ArduPhone - ui
 *
 * Functions related to running the user interface.
 *
 * Cary Dreelan - Dec 2013
 */


// === Constants ===
// Startup animation values
const unsigned int STARTUP_FRAME_PERIOD = 55 ; // How long between each animation frame
const unsigned int STARTUP_HOLD_DELAY = 1500 ; // How long to wait after animation completes
const byte STARTUP_NUM_FRAMES = 38 ;
const byte STARTUP_TEXT_HEIGHT = 12 ;
const Colour STARTUP_BG_COLOUR = { 0, 0, 2 } ;
const Colour STARTUP_FG_COLOUR = YELLOW ;
// Main menu values
const Colour MAIN_MENU_BG_COLOUR = NAVY ;
const Colour MAIN_MENU_FG_COLOUR = WHITE ;
const byte MAIN_MENU_MAX_LINES = 7 ;
const byte MAIN_MENU_LINE_HEIGHT = 16 ;
const byte MENU_ITEM_TEXT_BUF_LEN = 32 ;

// === States ===

enum uiStates {
  UI_UNINITIALISED,
  UI_START_UP,
  UI_DRAW_MAIN_MENU,
  UI_MAIN_MENU
} ;
uiStates uiState ;

// === Variables ===

unsigned long nextUITime ;
byte startupFrameNum = 0 ;
// Main menu variables - these next 3 items and their arrays must be in sync
prog_char mainMenu0[] PROGMEM = "Call";
prog_char mainMenu1[] PROGMEM = "SMS";
prog_char mainMenu2[] PROGMEM = "Lock";
prog_char mainMenu3[] PROGMEM = "Three";
prog_char mainMenu4[] PROGMEM = "Four";
prog_char mainMenu5[] PROGMEM = "Five";
prog_char mainMenu6[] PROGMEM = "Six";
prog_char mainMenu7[] PROGMEM = "Seven";
prog_char mainMenu8[] PROGMEM = "Eight";
prog_char mainMenu9[] PROGMEM = "Nine";
const byte MAIN_MENU_NUM_ITEMS = 10 ;
PROGMEM const char * main_menu_table[] = {
  mainMenu0,
  mainMenu1,
  mainMenu2,
  mainMenu3,
  mainMenu4,
  mainMenu5,
  mainMenu6,
  mainMenu7,
  mainMenu8,
  mainMenu9
} ;
typedef void ( * FunctionPointer ) ();
PROGMEM FunctionPointer main_menu_functions[] = {
  callMenuItem,
  smsMenuItem,
  lockMenuItem,
  0,
  0,
  0,
  0,
  0,
  0,
  0
} ;
byte curMenuItem, lastMenuItem = 0 ;

// === Functions ===

void callMenuItem() {
}
void smsMenuItem() {
}
void lockMenuItem() {
}

void drawMenuLine( const String menuText, const byte lineNum, const boolean selected ) {
  Colour fgColour = MAIN_MENU_FG_COLOUR ;
  Colour bgColour = MAIN_MENU_BG_COLOUR ;
  if ( selected ) {
    fgColour = MAIN_MENU_BG_COLOUR ;
    bgColour = MAIN_MENU_FG_COLOUR ;
  }
  oled.drawString( MF_MIN_X + 1, MF_MAX_Y - ( ( lineNum + 1 ) * MAIN_MENU_LINE_HEIGHT ), menuText, fgColour, bgColour ) ;
}

String getMenuItemText ( const byte itemNum ) {
  char itemText [ MENU_ITEM_TEXT_BUF_LEN ] ;
  strcpy_P ( itemText, ( char * ) pgm_read_word ( & ( main_menu_table[ itemNum ] ) ) ) ;
  return itemText ;
}

void drawMainMenu() {
  // Menu consists of 7 lines (MAIN_MENU_MAX_LINES) at 16 pixels (MAIN_MENU_LINE_HEIGHT) high
  // If more than 7 items exist in menu, start drawing from offsetItem to effectively scroll
  oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, MAIN_MENU_BG_COLOUR ) ;
  oled.selectFont( Arial_Black_16 ) ;
  byte offsetItem = 0 ;
  if ( curMenuItem >= MAIN_MENU_MAX_LINES ) {
    offsetItem = curMenuItem - MAIN_MENU_MAX_LINES + 1 ;
  }
  for ( byte lineNum = 0 ; lineNum < MAIN_MENU_MAX_LINES ; lineNum ++ ) {
    if ( lineNum + offsetItem < MAIN_MENU_NUM_ITEMS ) {
      drawMenuLine( getMenuItemText ( lineNum + offsetItem ), lineNum, curMenuItem == lineNum + offsetItem ) ;
    }
  }
  lastMenuItem = curMenuItem ; // track last selected menu item
}

// Draw if something changed
void updateMainMenu() {
  if ( curMenuItem != lastMenuItem ) {
   if (  curMenuItem >= MAIN_MENU_MAX_LINES || lastMenuItem >= MAIN_MENU_MAX_LINES ) {
     // Just redraw whole thing as we're scrolling items
     drawMainMenu() ;
   } else {
     // Just draw the items that have changed
     oled.selectFont( Arial_Black_16 ) ;
     drawMenuLine( getMenuItemText ( lastMenuItem ), lastMenuItem, false ) ;
     drawMenuLine( getMenuItemText ( curMenuItem ), curMenuItem, true ) ;
     lastMenuItem = curMenuItem ;
   }
  }
}

// Power/Start up animation
void startupAnimation() {
  if ( sliceStartTime >= nextUITime ) {
    if ( startupFrameNum == 0 ) {
      // First animation frame
      oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, STARTUP_BG_COLOUR ) ;
      oled.selectFont( Droid_Sans_12 ) ;
    } else if ( startupFrameNum == STARTUP_NUM_FRAMES ) {
      // Last animation frame
      // Unfortunately lots of magic values here to accurately draw oval box around freetronics logo
      oled.drawCircle( 21, MF_MAX_Y - 5 - startupFrameNum, 8, STARTUP_FG_COLOUR ) ;
      oled.drawCircle( 70, MF_MAX_Y - 5 - startupFrameNum, 8, STARTUP_FG_COLOUR ) ;
      oled.drawBox( 23, MF_MAX_Y - STARTUP_TEXT_HEIGHT - 1 - startupFrameNum, 70, MF_MAX_Y + 3 - startupFrameNum, 1, STARTUP_FG_COLOUR ) ;
      oled.drawFilledBox( 23, MF_MAX_Y - 1 - startupFrameNum, 70, MF_MAX_Y + 2 - startupFrameNum, STARTUP_BG_COLOUR ) ;
      oled.drawString( 20, MF_MAX_Y - STARTUP_TEXT_HEIGHT - startupFrameNum, F("freetronics"), STARTUP_FG_COLOUR, STARTUP_BG_COLOUR ) ;
      nextUITime = sliceStartTime + STARTUP_HOLD_DELAY ;
    } else if ( startupFrameNum == STARTUP_NUM_FRAMES + 1 ) {
      // Transition to menu
      uiState = UI_DRAW_MAIN_MENU ;
    } else {
      oled.drawLine( 20, MF_MAX_Y - startupFrameNum, 100, MF_MAX_Y - startupFrameNum, STARTUP_BG_COLOUR ) ; // Clear top of previous drawString
      oled.drawString( 20, MF_MAX_Y - STARTUP_TEXT_HEIGHT - startupFrameNum, F("freetronics"), STARTUP_FG_COLOUR, STARTUP_BG_COLOUR ) ;
      oled.drawString( 60, MF_MIN_Y + startupFrameNum, F("ArduPhone"), STARTUP_FG_COLOUR, STARTUP_BG_COLOUR ) ;
      nextUITime = sliceStartTime + STARTUP_FRAME_PERIOD ;
    }
    startupFrameNum ++ ;
  }
}

// Called from keypad 'module'
void handleKeyPressed( char key ) {
  switch ( uiState ) {
    
    case UI_MAIN_MENU :
      if ( key == 'D' ) {
        curMenuItem = ( curMenuItem + 1 ) % MAIN_MENU_NUM_ITEMS ;
        updateMainMenu() ;
      }
      if ( key == 'U' ) {
        if ( curMenuItem == 0 ) {
          curMenuItem = MAIN_MENU_NUM_ITEMS - 1 ;
        } else {
          curMenuItem -- ;
        }
        updateMainMenu() ;
      }
      break ;
      
    // Else ignore the key
  }
}

// =======================

void UISetup() {
  uiState = UI_UNINITIALISED ;
  nextUITime = sliceStartTime + 200 ; // wait breifly before starting UI
}

// =======================

void UISlice() {
  switch ( uiState ) {
    
    case UI_UNINITIALISED :
      uiState = UI_START_UP ;
      break ;
      
    case UI_START_UP :
      startupAnimation() ;
      break ;
      
    case UI_DRAW_MAIN_MENU :
      drawMainMenu() ;
      uiState = UI_MAIN_MENU ;
      break ;

    case UI_MAIN_MENU :
      break ;
  }
}