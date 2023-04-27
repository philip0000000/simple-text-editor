/* A Simple Text Editor for Linux Terminal and Windows Command Prompt.
 * Author philip0000000
 * MIT license
 *
 * g++ for linux
 * vc++ for windows
 */

#include <iostream>
#include <fstream> MIT license 
#include <vector>
#include <string>
#include <iterator>

// ===( global variables )===
std::string g_FileName;
std::vector<std::string> g_File;

// Y
int g_PrintFromLine = 0;
int g_CursorLine = 0;
int g_height = 0;

// X
int g_PrintFromX = 0;
int g_CursorPositionX = 0;
int g_width = 0;

enum ActionKey
{
    ENTER = 13,
    ESC_KEY = 27,
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN,
    CTRL_S = 3333,
    NO_INPUT = 3332
};

/**************************************************************************/

#if defined (_WIN32) // windows

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HANDLE hStdio;
HANDLE hStdout;

COORD tl = { 0, 0 };
CONSOLE_SCREEN_BUFFER_INFO s;
DWORD written, cells;

// for cursor
#define ASCII_EXTENDED_SQUARE (unsigned char)219
COORD g_posCursor = { 0, 0 };
DWORD BytesWritten = 0;

void Initialize()
{
    // get console handle
    hStdio = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdio == NULL)
    {
        std::cerr << "ERROR! could not find stdin from console!";
        exit(1);
        //return false; // console input not found
    }
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout == NULL)
    {
        std::cerr << "ERROR! could not find stdout from console!";
        exit(1);
        //return false; // console output not found
    }

    // hide console cursor
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hStdout, &info);

    // get info about console for when ClearScreen function
    GetConsoleScreenBufferInfo(hStdout, &s);
    cells = s.dwSize.X * s.dwSize.Y;

    // get terminal window columns and rows size
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    g_width = csbi.srWindow.Right - csbi.srWindow.Left;
    g_height = csbi.srWindow.Bottom - csbi.srWindow.Top;
}

void SetCursorPosition(int x, int y)
{
    SetConsoleCursorPosition(hStdout, g_posCursor);

    COORD pos = { (SHORT)x, (SHORT)y };
    if (FillConsoleOutputCharacterA(hStdout, ASCII_EXTENDED_SQUARE, 1, pos, &BytesWritten) == 0)
        exit(1);
}

int GetKey()
{
    DWORD cc;
    INPUT_RECORD irec;

    // loop until key presse
    do {
        ReadConsoleInput(hStdio, &irec, 1, &cc);
    } while (irec.EventType != KEY_EVENT);

    KEY_EVENT_RECORD ker = ((KEY_EVENT_RECORD&)irec.Event.KeyEvent);

    if (ker.bKeyDown == 0) // only if key is down act
        return ActionKey::NO_INPUT;

    switch (ker.wVirtualKeyCode)
    {
        case 8:
            return ActionKey::BACKSPACE;
            break;
        case 13:
            return ActionKey::ENTER;
            break;
        case 27:
            return ActionKey::ESC_KEY;
            break;
        case 33:
            return ActionKey::PAGE_UP;
            break;
        case 34:
            return ActionKey::PAGE_DOWN;
            break;
        case 37:
            return ActionKey::ARROW_LEFT;
            break;
        case 38:
            return ActionKey::ARROW_UP;
            break;
        case 39:
            return ActionKey::ARROW_RIGHT;
            break;
        case 40:
            return ActionKey::ARROW_DOWN;
            break;
        default:
            break;
    };

    if (ker.dwControlKeyState == 8) // ctrl keyi
    {
        if (ker.wVirtualKeyCode == 83) // s key
            return ActionKey::CTRL_S;
    }

    return ker.uChar.AsciiChar;
}

void ClearScreen()
{
    FillConsoleOutputCharacter(hStdout, ' ', cells, tl, &written);
    FillConsoleOutputAttribute(hStdout, s.wAttributes, cells, tl, &written);
}

#define READ_Ä -60
#define READ_Ö -42
#define READ_Å -59
#define READ_ä -28
#define READ_ö -10
#define READ_å -27
#define PRINT_Ä (unsigned int)142
#define PRINT_Ö (unsigned int)153
#define PRINT_Å (unsigned int)143
#define PRINT_ä (unsigned int)132
#define PRINT_ö (unsigned int)148
#define PRINT_å (unsigned int)134
void ParseStr(std::string &str)
{
    for (int i = 0; i < str.length(); i++)
    {
        switch (str[i])
        {
            case READ_Ä:
                str[i] = PRINT_Ä;
                break;
            case READ_Ö:
                str[i] = PRINT_Ö;
                break;
            case READ_Å:
                str[i] = PRINT_Å;
                break;
            case READ_ä:
                str[i] = PRINT_ä;
                break;
            case READ_ö:
                str[i] = PRINT_ö;
                break;
            case READ_å:
                str[i] = PRINT_å;
                break;
            default:
                break;
        }
    }
}

#else // linux

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// (ANSI escape sequences/VT100 codes) https://en.wikipedia.org/wiki/ANSI_escape_code

void ClearScreenCursorToHome()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
}

void ReturningCursorHomePosition()
{
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void ClearScreen()
{
    ClearScreenCursorToHome();
    ReturningCursorHomePosition();
}

struct termios g_termios;

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_termios) == -1)
    {
        std::cerr << "ERROR! can not disable raw mode.";
        exit(1);
    }
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &g_termios) == -1)
    {
        std::cerr << "ERROR! can not enable raw mode.";
        exit(1);
    }
    atexit(disableRawMode);

    struct termios raw = g_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        std::cerr << "ERROR! can not enable raw mode.";
        exit(1);
    }
}

int GetKey()
{
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
        {
            std::cerr << "ERROR! could not read file.";
            exit(1);
        }
    }

    if (c == '\x1b')
    {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';

        if (seq[0] == '[')
        {
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return '\x1b';
                if (seq[2] == '~')
                {
                    switch (seq[1])
                    {
                    case '5':
                        return ActionKey::PAGE_UP;
                        break;
                    case '6':
                        return ActionKey::PAGE_DOWN;
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                switch (seq[1])
                {
                case 'A':
                    return ActionKey::ARROW_UP;
                    break;
                case 'B':
                    return ActionKey::ARROW_DOWN;
                    break;
                case 'C':
                    return ActionKey::ARROW_RIGHT;
                    break;
                case 'D':
                    return ActionKey::ARROW_LEFT;
                    break;
                default:
                    break;
                }
            }
        }
        return '\x1b';
    }

    if (('s' & 0x1f) == c) // ctrl + s
        return ActionKey::CTRL_S;

    return c;
}

int getCursorPosition(int* rows, int* cols)
{
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;

    while (i < sizeof(buf) - 1)
    {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}

int getWindowSize(int* rows, int* cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return getCursorPosition(rows, cols);
    }
    else
    {
        // linux console, add +1, ... maybe?
        //ws.ws_col++;
        //ws.ws_row;

        *cols = ws.ws_col;
        *rows = ws.ws_row;

        return 0;
    }
}

void SetCursorPosition(int x, int y)
{
    // move cursor position and show cursor
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH\x1b[?25h", y + 1,// + 1,
        x + 1);
    // moves cursor to row n, column m (CSI n ; m H)
    // "\x1b[?25h" // show the cursor
    write(STDOUT_FILENO, buf, sizeof(buf));
}

void Initialize()
{
    enableRawMode();

    // get terminal window columns and rows size
    getWindowSize(&g_height, &g_width);
}

void ParseStr(std::string &str)
{
}

#endif

/**************************************************************************/

void ReadFile()
{
    std::ifstream fileIn(g_FileName.c_str());
    std::string str;

    while (std::getline(fileIn, str))
    {
        if (str.size() > 0) {
            g_File.push_back(str);
            g_File.back() += '\r';
        }
        else
            g_File.push_back("\r"); // new line
    }

    // Add empty string if the file is empty
    if (g_File.size() == 0)
        g_File.push_back(std::string());

    fileIn.close();
}

void SaveFile()
{
    std::ofstream FileOutput(g_FileName.c_str());
    std::ostream_iterator<std::string> outputIterator(FileOutput, "\n");
    std::copy(g_File.begin(), g_File.end(), outputIterator);
}

void RefreshScreen()
{
    if (g_CursorPositionX > g_PrintFromX + g_width - 1)
        g_PrintFromX++;
    else if (g_CursorPositionX < g_PrintFromX)
        g_PrintFromX--;

    ClearScreen();
    // Print screen
    for (int n = 0; n < g_File.size() && n < g_height - 1; n++)
    {
        if (g_File[g_PrintFromLine + n].size() > g_PrintFromX)
        {
            std::string str = g_File[g_PrintFromLine + n].substr(g_PrintFromX, g_width - 1);
            ParseStr(str);
            std::cout << str;
        }
        std::cout << "\n";
    }
    SetCursorPosition(g_CursorPositionX, g_CursorLine - g_PrintFromLine);
}

bool ProcessKeypress()
{
    int c = GetKey();

    switch (c)
    {
        case NO_INPUT:
            break;
        case ESC_KEY:
            ClearScreen();
            return false;
            break;
        case ARROW_UP:
            if (g_CursorLine > 0)
            {
                g_CursorLine--;
                if (g_CursorLine < g_PrintFromLine)
                    g_PrintFromLine--;
                // if new line is bigger, make x position smaller
                if (g_CursorPositionX > g_File[g_CursorLine].length() - 1)
                    g_CursorPositionX = (int)(g_File[g_CursorLine].length() - 1);
            }
            break;
        case ARROW_DOWN:
            if (g_CursorLine < g_File.size() - 1)
            {
                g_CursorLine++;
                if (g_CursorLine > g_PrintFromLine + g_height - 2)
                    g_PrintFromLine++;
                // if new line is bigger, make x position smaller
                if (g_CursorPositionX > g_File[g_CursorLine].length() - 1)
                    g_CursorPositionX = (int)(g_File[g_CursorLine].length() - 1);
            }
            break;
        case ARROW_LEFT:
            if (g_CursorPositionX > 0)
                g_CursorPositionX--;
            else if (g_CursorPositionX == 0)
            {
                // go to end of line above, if it exist
                if (g_CursorLine > 0)
                {
                    g_CursorLine--;
                    if (g_CursorLine < g_PrintFromLine)
                        g_PrintFromLine--;
                    g_CursorPositionX = (int)(g_File[g_CursorLine].length() - 1);
                }
            }
            break;
        case ARROW_RIGHT:
            if (g_CursorPositionX < g_File[g_CursorLine].length() - 1)
            {
                g_CursorPositionX++;
            }
            else
            {
                // go to next line, if it exist
                if (g_CursorLine < g_File.size() - 1)
                {
                    g_CursorLine++;
                    if (g_CursorLine > g_PrintFromLine + g_height - 2)
                        g_PrintFromLine++;
                    g_CursorPositionX = 0;
                }
            }
            break;
        case PAGE_UP:
            g_PrintFromLine -= g_height;
            if (g_PrintFromLine < 0)
                g_PrintFromLine = 0;
            g_CursorLine -= g_height;
            if (g_CursorLine < 0)
                g_CursorLine = 0;
            // if new line is bigger, make x position smaller
            if (g_CursorPositionX > g_File[g_CursorLine].length() - 1)
                g_CursorPositionX = (int)(g_File[g_CursorLine].length() - 1);
            break;
        case PAGE_DOWN:
            g_PrintFromLine += g_height;
            if (g_PrintFromLine > g_File.size() - g_height || g_PrintFromLine > g_File.size() - 1)
            {
                g_PrintFromLine = (int)(g_File.size() - g_height);
                if (g_PrintFromLine < 0)
                    g_PrintFromLine = (int)(g_File.size() - 1);
            }
            g_CursorLine += g_height;
            if (g_CursorLine > g_File.size() - 1)
                g_CursorLine = (int)(g_File.size() - 1);
            // if new line is bigger, make x position smaller
            if (g_CursorPositionX > g_File[g_CursorLine].length() - 1)
                g_CursorPositionX = (int)(g_File[g_CursorLine].length() - 1);
            break;
        case BACKSPACE:
            if (g_CursorPositionX == 0)
            {
                if (g_CursorLine > 0)
                {
                    g_CursorPositionX = (int)(g_File[g_CursorLine - 1].length() - 1); // set posistion
                    g_File[g_CursorLine - 1].pop_back(); // remove '\r'
                    g_File[g_CursorLine - 1] += g_File[g_CursorLine]; // add deleted line
                    g_File.erase(g_File.begin() + g_CursorLine); // delete line
                    g_CursorLine--; // cursor move up one
                }
            }
            else if (g_CursorPositionX < g_File[g_CursorLine].length())
            {
                // delete character
                g_File[g_CursorLine].erase(g_CursorPositionX, 1) + '\r';
                g_CursorPositionX--;
            }
            if (g_CursorLine < g_PrintFromLine)
                g_PrintFromLine--;
            break;
        case ENTER:
            if (g_CursorPositionX == 0)
            {
                g_File.insert(g_File.begin() + g_CursorLine, std::string("\r"));
                g_CursorPositionX = 0;
                g_CursorLine++;
            }
            else if (g_CursorPositionX == g_File[g_CursorLine].length() - 1)
            {
                g_File.insert(g_File.begin() + g_CursorLine + 1, std::string("\r"));
                g_CursorPositionX = 0;
                g_CursorLine++;
            }
            else
            {
                // add new line, and add string
                g_File.insert(g_File.begin() + g_CursorLine + 1,
                    g_File[g_CursorLine].substr(g_CursorPositionX));
                // remove from string, what was added
                g_File[g_CursorLine] = g_File[g_CursorLine].erase(g_CursorPositionX,
                    g_File[g_CursorLine].length() - g_CursorPositionX - 1);
                // set new posistion for new line
                g_CursorPositionX = 0;
                g_CursorLine++;
            }
            if (g_CursorLine > g_PrintFromLine + g_height - 2)
                g_PrintFromLine++;
            break;
        case CTRL_S: // save
            SaveFile();
            break;
        default: // add character
            if (c != 0)
            {
                g_File[g_CursorLine].insert(g_CursorPositionX, 1, c);
                g_CursorPositionX++;
            }
            break;
    }

    return true;
}

int main(int argc, char* argv[])
{
    if (argc != 2) // if error
    {
        std::cerr << "ERROR! no file to open" << std::endl;
        return 1;
    }

    g_FileName.append(argv[1]); // save file name
    ReadFile();                 // read file
    Initialize();

    // main loop
    bool NotExit = true;
    while (NotExit == true)
    {
        RefreshScreen();
        NotExit = ProcessKeypress();
    }

    return 0;
}
