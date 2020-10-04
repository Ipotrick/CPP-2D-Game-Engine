#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

enum class BUTTON {
	MB_LEFT = 0,
	MB_RIGHT = 1,
	MB_MIDDLE = 2,
	MB_4 = 3,
	MB_5 = 4
};

enum class KEY {

SPACE             = 32,
APOSTROPHE        = 39,  /* ' */
COMMA             = 44,  /* , */
MINUS             = 45,  /* - */
PERIOD            = 46,  /* . */
SLASH             = 47,  /* / */
ZERO              = 48,
ONE               = 49,
TWO               = 50,
THREE             = 51,
FOUR              = 52,
FIFE              = 53,
SIX               = 54,
SEVEN             = 55,
EIGHT             = 56,
NINE              = 57,
SEMICOLON         = 59,  /* ; */
EQUAL             = 61,  /* = */
A                 = 65,
B                 = 66,
C                 = 67,
D                 = 68,
E                 = 69,
F                 = 70,
G                 = 71,
H                 = 72,
I                 = 73,
J                 = 74,
K                 = 75,
L                 = 76,
M                 = 77,
N                 = 78,
O                 = 79,
P                 = 80,
Q                 = 81,
R                 = 82,
S                 = 83,
T                 = 84,
U                 = 85,
V                 = 86,
W                 = 87,
X                 = 88,
Y                 = 89,
Z                 = 90,
LEFT_BRACKET      = 91,  /* [ */
BACKSLASH         = 92,  /* \ */
RIGHT_BRACKET     = 93,  /* ] */
GRAVE_ACCENT      = 96,  /* ` */
WORLD_1           = 161, /* non-US #1 */
WORLD_2           = 162, /* non-US #2 */

	/* Function keys */
ESCAPE            = 256,
ENTER             = 257,
TAB               = 258,
BACKSPACE         = 259,
INSERT            = 260,
DELETE            = 261,
RIGHT             = 262,
LEFT              = 263,
DOWN              = 264,
UP                = 265,
PAGE_UP           = 266,
PAGE_DOWN         = 267,
HOME              = 268,
END               = 269,
CAPS_LOCK         = 280,
SCROLL_LOCK       = 281,
NUM_LOCK          = 282,
PRINT_SCREEN      = 283,
PAUSE             = 284,
F1                = 290,
F2                = 291,
F3                = 292,
F4                = 293,
F5                = 294,
F6                = 295,
F7                = 296,
F8                = 297,
F9                = 298,
F10               = 299,
F11               = 300,
F12               = 301,
F13               = 302,
F14               = 303,
F15               = 304,
F16               = 305,
F17               = 306,
F18               = 307,
F19               = 308,
F20               = 309,
F21               = 310,
F22               = 311,
F23               = 312,
F24               = 313,
F25               = 314,
NP_0              = 320,
NP_1              = 321,
NP_2              = 322,
NP_3              = 323,
NP_4              = 324,
NP_5              = 325,
NP_6              = 326,
NP_7              = 327,
NP_8              = 328,
NP_9              = 329,
NP_DECIMAL        = 330,
NP_DIVIDE         = 331,
NP_MULTIPLY       = 332,
NP_SUBTRACT       = 333,
NP_ADD            = 334,
NP_ENTER          = 335,
NP_EQUAL          = 336,
LEFT_SHIFT        = 340,
LEFT_CONTROL      = 341,
LEFT_ALT          = 342,
LEFT_SUPER        = 343,
RIGHT_SHIFT       = 344,
RIGHT_CONTROL     = 345,
RIGHT_ALT         = 346,
RIGHT_SUPER       = 347,
MENU              = 348

};

inline static constexpr int MAX_KEY_INDEX = 348;
inline static constexpr int MIN_KEY_INDEX = 32;

inline static constexpr char KEY_UP = 0;
inline static constexpr char KEY_DOWN = 1;
inline static constexpr char KEY_REPEAT = 2;

enum class InputStatus {
	UP = GLFW_RELEASE,
	DOWN = GLFW_PRESS,
	REPEAT = GLFW_REPEAT
};