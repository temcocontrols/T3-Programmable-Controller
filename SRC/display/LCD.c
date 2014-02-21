#include "main.h"
#include "lcd.h"
#include  <intrins.h> 
#include <string.h>
#include <ctype.h>   



unsigned char const code aby_Char_Table[10] = {'0','1','2','3','4','5','6','7','8','9'};

/*(0) !(1) "(2) #(3) $(4) %(5) &(6) '(7) ((8) )(9) *(10) +(11) ,(12) -(13) .(14) /(15)
 0(16) 1(17) 2(18) 3(19) 4(20) 5(21) 6(22) 7(23) 8(24) 9(25) :(26) ;(27) <(28) =(29) >(30) ?(31)
 @(32) A(33) B(34) C(35) D(36) E(37) F(38) G(39) H(40) I(41) J(42) K(43) L(44) M(45) N(46) O(47)
 P(48) Q(49) R(50) S(51) T(52) U(53) V(54) W(55) X(56) Y(57) Z(58) [(59) \(60) ](61) ^(62) _(63)
 `(64) a(65) b(66) c(67) d(68) e(69) f(70) g(71) h(72) i(73) j(74) k(75) l(76) m(77) n(78) o(79)
 p(80) q(81) r(82) s(83) t(84) u(85) v(86) w(87) x(88) y(89) z(90) {(91) |(92) }(93) ~(94) |(95)*/ 




unsigned char const code Dot8_16[95][2][8] = 
{
	{
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},/*" ",0*/
	},
	{
		{0xFF,0xFF,0xFF,0x03,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xF3,0xFF,0xFF,0xFF,0xFF},/*"!",1*/
	},
	{
		{0xFF,0xEF,0xF3,0xEF,0xF3,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},/*""",2*/
	},
	{
		{0xFF,0xEF,0x03,0xEF,0x03,0xEF,0xFF,0xFF},
		{0xFF,0xFB,0xE0,0xFB,0xE0,0xFB,0xFF,0xFF},/*"#",3*/
	},
	{
		{0xFF,0xCF,0xB7,0x03,0x77,0xCF,0xFF,0xFF},
		{0xFF,0xF9,0xF7,0xE0,0xF7,0xF8,0xFF,0xFF},/*"$",4*/
	},
	{
		{0xFF,0xE7,0xDB,0x27,0xCF,0xF3,0xFF,0xFF},
		{0xFF,0xF3,0xFC,0xF9,0xF6,0xF9,0xFF,0xFF},/*"%",5*/
	},
	{
		{0xFF,0x67,0x9B,0x67,0xFF,0x7F,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF6,0xF9,0xF6,0xFF,0xFF},/*"&",6*/
	},
	{
		{0xFF,0xFF,0xEB,0xF3,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},/*"'",7*/
	},
	{
		{0xFF,0xFF,0x1F,0xE7,0xFB,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFC,0xF3,0xEF,0xFF,0xFF,0xFF},/*"(",8*/
	},
	{
		{0xFF,0xFF,0xFB,0xE7,0x1F,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xEF,0xF3,0xFC,0xFF,0xFF,0xFF},/*")",9*/
	},
	{
		{0xFF,0x9F,0x7F,0x0F,0x7F,0x9F,0xFF,0xFF},
		{0xFF,0xFC,0xFF,0xF8,0xFF,0xFC,0xFF,0xFF},/*"*",10*/
	},
	{
		{0xFF,0x7F,0x7F,0x0F,0x7F,0x7F,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xF8,0xFF,0xFF,0xFF,0xFF},/*"+",11*/
	},
	{
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xEB,0xF3,0xFF,0xFF,0xFF,0xFF},/*",",12*/
	},
	{
		{0xFF,0x7F,0x7F,0x7F,0x7F,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},/*"-",13*/
	},
	{
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xF3,0xF3,0xFF,0xFF,0xFF,0xFF},/*".",14*/
	},
	{
		{0xFF,0xFF,0xFF,0x3F,0xCF,0xF3,0xFF,0xFF},
		{0xFF,0xE7,0xF9,0xFE,0xFF,0xFF,0xFF,0xFF},/*"/",15*/
	},
	{
		{0xFF,0x0F,0xF7,0xF7,0x0F,0xFF,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF8,0xFF,0xFF,0xFF},/*"0",16*/
	},
	{
		{0xFF,0xFF,0xEF,0x07,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0xFF},/*"1",17*/
	},
	{
		{0xFF,0xCF,0xF7,0x77,0x8F,0xFF,0xFF,0xFF},
		{0xFF,0xF3,0xF4,0xF7,0xF7,0xFF,0xFF,0xFF},/*"2",18*/
	},
	{
		{0xFF,0xCF,0x77,0x77,0x8F,0xFF,0xFF,0xFF},
		{0xFF,0xF9,0xF7,0xF7,0xF8,0xFF,0xFF,0xFF},/*"3",19*/
	},
	{
		{0xFF,0xFF,0x3F,0xCF,0x07,0xFF,0xFF,0xFF},
		{0xFF,0xFC,0xFD,0xFD,0xF0,0xFD,0xFF,0xFF},/*"4",20*/
	},
	{
		{0xFF,0x07,0xB7,0xB7,0x77,0xFF,0xFF,0xFF},
		{0xFF,0xF9,0xF7,0xF7,0xF8,0xFF,0xFF,0xFF},/*"5",21*/
	},
	{
		{0xFF,0x0F,0x77,0x77,0xCF,0xFF,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF8,0xFF,0xFF,0xFF},/*"6",22*/
	},
	{
		{0xFF,0xF7,0xF7,0x37,0xC7,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xF1,0xFE,0xFF,0xFF,0xFF,0xFF},/*"7",23*/
	},
	{
		{0xFF,0x8F,0x77,0x77,0x8F,0xFF,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF8,0xFF,0xFF,0xFF},/*"8",24*/
	},
	{
		{0xFF,0x8F,0x77,0x77,0x0F,0xFF,0xFF,0xFF},
		{0xFF,0xF9,0xF7,0xF7,0xF8,0xFF,0xFF,0xFF},/*"9",25*/
	},
	{
		{0xFF,0xFF,0x9F,0x9F,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xF3,0xF3,0xFF,0xFF,0xFF,0xFF},/*":",26*/
	},
	{
		{0xFF,0xFF,0x9F,0x9F,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xEB,0xF3,0xFF,0xFF,0xFF,0xFF},/*";",27*/
	},
	{
		{0xFF,0x7F,0xBF,0xDF,0xEF,0xF7,0xFF,0xFF},
		{0xFF,0xFF,0xFE,0xFD,0xFB,0xF7,0xFF,0xFF},/*"<",28*/
	},
	{
		{0xFF,0xDF,0xDF,0xDF,0xDF,0xFF,0xFF,0xFF},
		{0xFF,0xFE,0xFE,0xFE,0xFE,0xFF,0xFF,0xFF},/*"=",29*/
	},
	{
		{0xFF,0xF7,0xEF,0xDF,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xF7,0xFB,0xFD,0xFE,0xFF,0xFF,0xFF},/*">",30*/
	},
	{
		{0xFF,0xCF,0xF7,0x77,0x8F,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xF2,0xFF,0xFF,0xFF,0xFF},/*"?",31*/
	},
	{
		{0xFF,0x0F,0xD7,0x17,0xF7,0x0F,0xFF,0xFF},
		{0xFF,0xF8,0xF6,0xF6,0xF5,0xFA,0xFF,0xFF},/*"@",32*/
	},
	{
		{0xFF,0xFF,0x1F,0xE7,0x1F,0xFF,0xFF,0xFF},
		{0xFF,0xF0,0xFD,0xFD,0xFD,0xF0,0xFF,0xFF},/*"A",33*/
	},
	{
		{0xFF,0x07,0x77,0x77,0x77,0x8F,0xFF,0xFF},
		{0xFF,0xF0,0xF7,0xF7,0xF7,0xF8,0xFF,0xFF},/*"B",34*/
	},
	{
		{0xFF,0x0F,0xF7,0xF7,0xF7,0xCF,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF7,0xF9,0xFF,0xFF},/*"C",35*/
	},
	{
		{0xFF,0x07,0xF7,0xF7,0xEF,0x1F,0xFF,0xFF},
		{0xFF,0xF0,0xF7,0xF7,0xFB,0xFC,0xFF,0xFF},/*"D",36*/
	},
	{
		{0xFF,0x07,0x77,0x77,0x77,0xF7,0xFF,0xFF},
		{0xFF,0xF0,0xF7,0xF7,0xF7,0xF7,0xFF,0xFF},/*"E",37*/
	},
	{
		{0xFF,0x07,0x77,0x77,0x77,0xF7,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},/*"F",38*/
	},
	{
		{0xFF,0x0F,0xF7,0xF7,0xF7,0xCF,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xFA,0xF0,0xFF,0xFF},/*"G",39*/
	},
	{
		{0xFF,0x07,0x7F,0x7F,0x7F,0x07,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xFF,0xFF,0xF0,0xFF,0xFF},/*"H",40*/
	},
	{
		{0xFF,0xFF,0xF7,0x07,0xF7,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xF7,0xF0,0xF7,0xFF,0xFF,0xFF},/*"I",41*/
	},
	{
		{0xFF,0xFF,0xFF,0xFF,0x07,0xFF,0xFF,0xFF},
		{0xFF,0xF9,0xF7,0xF7,0xF8,0xFF,0xFF,0xFF},/*"J",42*/
	},
	{
		{0xFF,0x07,0x7F,0x9F,0xE7,0xFF,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xFC,0xF3,0xFF,0xFF,0xFF},/*"K",43*/
	},
	{
		{0xFF,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xF0,0xF7,0xF7,0xF7,0xF7,0xFF,0xFF},/*"L",44*/
	},
	{
		{0xFF,0x07,0x1F,0xFF,0x1F,0x07,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xF0,0xFF,0xF0,0xFF,0xFF},/*"M",45*/
	},
	{
		{0xFF,0x07,0xCF,0x3F,0xFF,0x07,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xFE,0xF9,0xF0,0xFF,0xFF},/*"N",46*/
	},
	{
		{0xFF,0x0F,0xF7,0xF7,0xF7,0x0F,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF7,0xF8,0xFF,0xFF},/*"O",47*/
	},
	{
		{0xFF,0x07,0x77,0x77,0x77,0x8F,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},/*"P",48*/
	},
	{
		{0xFF,0x0F,0xF7,0xF7,0xF7,0x0F,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF5,0xFB,0xF4,0xFF,0xFF},/*"Q",49*/
	},
	{
		{0xFF,0x07,0x77,0x77,0x77,0x8F,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xFF,0xFE,0xF1,0xFF,0xFF},/*"R",50*/
	},
	{
		{0xFF,0xCF,0xB7,0x77,0xF7,0xCF,0xFF,0xFF},
		{0xFF,0xF9,0xF7,0xF7,0xF6,0xF9,0xFF,0xFF},/*"S",51*/
	},
	{
		{0xFF,0xF7,0xF7,0x07,0xF7,0xF7,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0xFF},/*"T",52*/
	},
	{
		{0xFF,0x07,0xFF,0xFF,0xFF,0x07,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF7,0xF8,0xFF,0xFF},/*"U",53*/
	},
	{
		{0xFF,0xC7,0x3F,0xFF,0x3F,0xC7,0xFF,0xFF},
		{0xFF,0xFF,0xFE,0xF1,0xFE,0xFF,0xFF,0xFF},/*"V",54*/
	},
	{
		{0xFF,0x07,0xFF,0x07,0xFF,0x07,0xFF,0xFF},
		{0xFF,0xFF,0xF0,0xFF,0xF0,0xFF,0xFF,0xFF},/*"W",55*/
	},
	{
		{0xFF,0xE7,0x9F,0x7F,0x9F,0xE7,0xFF,0xFF},
		{0xFF,0xF3,0xFC,0xFF,0xFC,0xF3,0xFF,0xFF},/*"X",56*/
	},
	{
		{0xFF,0xE7,0x9F,0x7F,0x9F,0xE7,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0xFF},/*"Y",57*/
	},
	{
		{0xFF,0xF7,0xF7,0x77,0x97,0xE7,0xFF,0xFF},
		{0xFF,0xF3,0xF4,0xF7,0xF7,0xF7,0xFF,0xFF},/*"Z",58*/
	},
	{
		{0xFF,0xFF,0xFF,0x03,0xFB,0xFB,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xE0,0xEF,0xEF,0xFF,0xFF},/*"[",59*/
	},
	{
		{0xFF,0xF3,0xCF,0x3F,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xFE,0xF9,0xE7,0xFF,0xFF},/*"\",60*/
	},
	{
		{0xFF,0xFB,0xFB,0x03,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xEF,0xEF,0xE0,0xFF,0xFF,0xFF,0xFF},/*"]",61*/
	},
	{
		{0xFF,0xFF,0xF7,0xFB,0xF7,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},/*"^",62*/
	},
	{
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
		{0xEF,0xEF,0xEF,0xEF,0xEF,0xEF,0xFF,0xFF},/*"_",63*/
	},
	{
		{0xFF,0xFF,0xFB,0xF7,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},/*"`",64*/
	},
	{
		{0xFF,0x7F,0xBF,0xBF,0x7F,0xFF,0xFF,0xFF},
		{0xFF,0xF9,0xF6,0xF6,0xF8,0xF7,0xFF,0xFF},/*"a",65*/
	},
	{
		{0xFF,0x07,0xBF,0xBF,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xF0,0xF7,0xF7,0xF7,0xF8,0xFF,0xFF},/*"b",66*/
	},
	{
		{0xFF,0x7F,0xBF,0xBF,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF7,0xFB,0xFF,0xFF},/*"c",67*/
	},
	{
		{0xFF,0x7F,0xBF,0xBF,0xBF,0x07,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF7,0xF0,0xFF,0xFF},/*"d",68*/
	},
	{
		{0xFF,0x7F,0xBF,0xBF,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xF8,0xF6,0xF6,0xF6,0xFA,0xFF,0xFF},/*"e",69*/
	},
	{
		{0xFF,0xBF,0x0F,0xB7,0xF7,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF},/*"f",70*/
	},
	{
		{0xFF,0x7F,0xBF,0xBF,0x7F,0xBF,0xFF,0xFF},
		{0xFF,0xF5,0xEA,0xEA,0xEB,0xF7,0xFF,0xFF},/*"g",71*/
	},
	{
		{0xFF,0x07,0xBF,0xBF,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xFF,0xFF,0xF0,0xFF,0xFF},/*"h",72*/
	},
	{
		{0xFF,0xFF,0xFF,0x27,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0xFF},/*"i",73*/
	},
	{
		{0xFF,0xFF,0xFF,0x27,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xEF,0xEF,0xF0,0xFF,0xFF,0xFF,0xFF},/*"j",74*/
	},
	{
		{0xFF,0x07,0xFF,0xFF,0x7F,0xBF,0xFF,0xFF},
		{0xFF,0xF0,0xFD,0xFC,0xFB,0xF7,0xFF,0xFF},/*"k",75*/
	},
	{
		{0xFF,0xFF,0xFF,0x07,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0xFF},/*"l",76*/
	},
	{
		{0xFF,0x3F,0xBF,0x7F,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xF0,0xFF,0xF0,0xFF,0xFF},/*"m",77*/
	},
	{
		{0xFF,0x3F,0xBF,0xBF,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xF0,0xFF,0xFF,0xFF,0xF0,0xFF,0xFF},/*"n",78*/
	},
	{
		{0xFF,0x7F,0xBF,0xBF,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF7,0xF8,0xFF,0xFF},/*"o",79*/
	},
	{
		{0xFF,0x3F,0xBF,0xBF,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xE0,0xFB,0xFB,0xFB,0xFC,0xFF,0xFF},/*"p",80*/
	},
	{
		{0xFF,0x7F,0xBF,0xBF,0xBF,0x3F,0xFF,0xFF},
		{0xFF,0xFC,0xFB,0xFB,0xFB,0xE0,0xFF,0xFF},/*"q",81*/
	},
	{
		{0xFF,0xFF,0x3F,0x7F,0xBF,0xBF,0xFF,0xFF},
		{0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF},/*"r",82*/
	},
	{
		{0xFF,0x7F,0xBF,0xBF,0xBF,0x7F,0xFF,0xFF},
		{0xFF,0xFB,0xF6,0xF6,0xF5,0xFB,0xFF,0xFF},/*"s",83*/
	},
	{
		{0xFF,0xBF,0x07,0xBF,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xF8,0xF7,0xF7,0xFF,0xFF,0xFF},/*"t",84*/
	},
	{
		{0xFF,0x3F,0xFF,0xFF,0xFF,0x3F,0xFF,0xFF},
		{0xFF,0xF8,0xF7,0xF7,0xF7,0xF0,0xFF,0xFF},/*"u",85*/
	},
	{
		{0xFF,0x3F,0xFF,0xFF,0xFF,0x3F,0xFF,0xFF},
		{0xFF,0xFF,0xFC,0xF3,0xFC,0xFF,0xFF,0xFF},/*"v",86*/
	},
	{
		{0xFF,0x3F,0xFF,0x3F,0xFF,0x3F,0xFF,0xFF},
		{0xFF,0xFE,0xF1,0xFE,0xF1,0xFE,0xFF,0xFF},/*"w",87*/
	},
	{
		{0xFF,0xBF,0x7F,0xFF,0x7F,0xBF,0xFF,0xFF},
		{0xFF,0xF7,0xFB,0xFC,0xFB,0xF7,0xFF,0xFF},/*"x",88*/
	},
	{
		{0xFF,0x3F,0xFF,0xFF,0xFF,0x3F,0xFF,0xFF},
		{0xFF,0xEF,0xEC,0xF3,0xFC,0xFF,0xFF,0xFF},/*"y",89*/
	},
	{
		{0xFF,0xBF,0xBF,0xBF,0xBF,0x3F,0xFF,0xFF},
		{0xFF,0xF7,0xF3,0xF5,0xF6,0xF7,0xFF,0xFF},/*"z",90*/
	},
	{
		{0xFF,0xFF,0x7F,0x83,0xFB,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xE0,0xEF,0xFF,0xFF,0xFF},/*"{",91*/
	},
	{
		{0xFF,0xFF,0xFF,0x01,0xFF,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xE0,0xFF,0xFF,0xFF,0xFF},/*"|",92*/
	},
	{
		{0xFF,0xFF,0xFB,0x83,0x7F,0xFF,0xFF,0xFF},
		{0xFF,0xFF,0xEF,0xE0,0xFF,0xFF,0xFF,0xFF},/*"}",93*/
	},
	{
		{0xFF,0xF7,0xFB,0xFB,0xF7,0xFB,0xFF,0xFF},
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},/*"~",94*/
	}	
};


unsigned char const code nAsciiDot[96][2][6] =  {

{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*" ",0*/

{0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x33,0x00,0x00,0x00},/*"!",1*/

{0x00,0x08,0x06,0x08,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*""",2*/

{0x40,0x40,0xF8,0x40,0xF8,0x40,0x04,0x3F,0x05,0x3F,0x05,0x04},/*"#",3*/

{0x70,0x88,0xFC,0x08,0x30,0x00,0x18,0x20,0xFF,0x21,0x1E,0x00},/*"$",4*/

{0xF0,0x98,0x60,0xE0,0x38,0x00,0x00,0x31,0x0E,0x1F,0x33,0x0C},/*"%",5*/

{0x00,0xF0,0x88,0x70,0x00,0x00,0x1E,0x21,0x26,0x19,0x27,0x20},/*"&",6*/

{0x10,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"'",7*/

{0x00,0x00,0x00,0xF0,0x0C,0x02,0x00,0x00,0x00,0x0F,0x30,0x40},/*"(",8*/

{0x00,0x06,0x18,0xE0,0x00,0x00,0x00,0x60,0x18,0x07,0x00,0x00},/*")",9*/

{0x40,0x80,0xF0,0x80,0x40,0x00,0x02,0x01,0x0F,0x01,0x02,0x00},/*"*",10*/

{0x00,0x00,0xF0,0x00,0x00,0x00,0x01,0x01,0x1F,0x01,0x01,0x00},/*"+",11*/

{0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x70,0x00,0x00,0x00,0x00},/*",",12*/

{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x00},/*"-",13*/

{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00},/*".",14*/

{0x00,0x00,0x80,0x70,0x0C,0x00,0x60,0x1C,0x03,0x00,0x00,0x00},/*"/",15*/

{0xF0,0x08,0x08,0x18,0xE0,0x00,0x1F,0x20,0x20,0x30,0x0F,0x00},/*"0",16*/

{0x00,0x10,0xF8,0x00,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00},/*"1",17*/

{0x70,0x08,0x08,0x08,0xF0,0x00,0x30,0x2C,0x22,0x21,0x30,0x00},/*"2",18*/

{0x30,0x08,0x88,0x88,0x70,0x00,0x18,0x20,0x20,0x31,0x1E,0x00},/*"3",19*/

{0x00,0x80,0x60,0xF0,0x00,0x00,0x06,0x05,0x24,0x3F,0x24,0x04},/*"4",20*/

{0xF8,0x08,0x88,0x88,0x08,0x00,0x19,0x21,0x20,0x31,0x1F,0x00},/*"5",21*/

{0xE0,0x18,0x88,0x98,0x00,0x00,0x0F,0x31,0x20,0x20,0x1F,0x00},/*"6",22*/

{0x38,0x08,0x08,0xE8,0x18,0x00,0x00,0x00,0x3F,0x00,0x00,0x00},/*"7",23*/

{0x70,0x88,0x08,0x88,0x70,0x00,0x1E,0x21,0x21,0x21,0x1E,0x00},/*"8",24*/

{0xF0,0x08,0x08,0x18,0xE0,0x00,0x01,0x32,0x22,0x31,0x0F,0x00},/*"9",25*/

{0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00},/*":",26*/

{0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00},/*";",27*/

{0x00,0x00,0x80,0x60,0x10,0x08,0x00,0x01,0x02,0x0C,0x10,0x20},/*"<",28*/

{0x40,0x40,0x40,0x40,0x40,0x00,0x04,0x04,0x04,0x04,0x04,0x00},/*"=",29*/

{0x00,0x08,0x30,0x40,0x80,0x00,0x00,0x20,0x18,0x04,0x02,0x01},/*">",30*/

{0x70,0x08,0x08,0x88,0x70,0x00,0x00,0x00,0x37,0x01,0x00,0x00},/*"?",31*/

{0xF0,0x10,0x88,0x78,0xF0,0x00,0x1F,0x20,0x27,0x34,0x0F,0x00},/*"@",32*/

{0x00,0x80,0x78,0xE0,0x00,0x00,0x20,0x3F,0x02,0x03,0x3E,0x20},/*"A",33*/

{0x08,0xF8,0x88,0x88,0x70,0x00,0x20,0x3F,0x20,0x20,0x1F,0x00},/*"B",34*/

{0xF0,0x10,0x08,0x08,0x18,0x00,0x1F,0x20,0x20,0x20,0x10,0x00},/*"C",35*/

{0x08,0xF8,0x08,0x10,0xE0,0x00,0x20,0x3F,0x20,0x10,0x0F,0x00},/*"D",36*/

{0x08,0xF8,0x88,0xE8,0x18,0x00,0x20,0x3F,0x20,0x23,0x30,0x00},/*"E",37*/

{0x08,0xF8,0x88,0xE8,0x18,0x00,0x20,0x3F,0x20,0x03,0x00,0x00},/*"F",38*/

{0xE0,0x10,0x08,0x08,0x18,0x00,0x0F,0x10,0x20,0x22,0x1E,0x02},/*"G",39*/

{0x08,0xF8,0x00,0x00,0xF8,0x08,0x20,0x3F,0x01,0x01,0x3F,0x20},/*"H",40*/

{0x08,0x08,0xF8,0x08,0x08,0x00,0x20,0x20,0x3F,0x20,0x20,0x00},/*"I",41*/

{0x00,0x08,0x08,0xF8,0x08,0x08,0xC0,0x80,0x80,0x7F,0x00,0x00},/*"J",42*/

{0x08,0xF8,0x88,0x60,0x18,0x08,0x20,0x3F,0x20,0x07,0x3C,0x20},/*"K",43*/

{0x08,0xF8,0x08,0x00,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x30},/*"L",44*/

{0xF8,0xF8,0x00,0xF8,0xF8,0x00,0x3F,0x00,0x3F,0x00,0x3F,0x00},/*"M",45*/

{0x08,0xF8,0xE0,0x08,0xF8,0x08,0x20,0x3F,0x21,0x06,0x3F,0x00},/*"N",46*/

{0xF0,0x08,0x08,0x18,0xE0,0x00,0x1F,0x20,0x20,0x30,0x0F,0x00},/*"O",47*/

{0x08,0xF8,0x08,0x08,0xF0,0x00,0x20,0x3F,0x21,0x01,0x00,0x00},/*"P",48*/

{0xF0,0x08,0x08,0x18,0xE0,0x00,0x0F,0x34,0x2C,0x70,0x7F,0x00},/*"Q",49*/

{0x08,0xF8,0x88,0x88,0x70,0x00,0x20,0x3F,0x20,0x03,0x3C,0x20},/*"R",50*/

{0x70,0x88,0x08,0x08,0x18,0x00,0x30,0x20,0x21,0x22,0x1C,0x00},/*"S",51*/

{0x18,0x08,0xF8,0x08,0x18,0x00,0x00,0x20,0x3F,0x20,0x00,0x00},/*"T",52*/

{0x08,0xF8,0x00,0x00,0xF8,0x08,0x00,0x1F,0x20,0x20,0x1F,0x00},/*"U",53*/

{0x08,0xF8,0x00,0x80,0xF8,0x08,0x00,0x03,0x3C,0x0F,0x00,0x00},/*"V",54*/

{0x78,0x84,0xF8,0xC0,0x78,0x00,0x00,0x3F,0x00,0x3F,0x00,0x00},/*"W",55*/

{0x08,0x78,0x80,0x78,0x08,0x00,0x20,0x3C,0x03,0x3C,0x20,0x00},/*"X",56*/

{0x08,0xF8,0x80,0x78,0x08,0x00,0x00,0x20,0x3F,0x20,0x00,0x00},/*"Y",57*/

{0x18,0x08,0x88,0x78,0x08,0x00,0x20,0x3C,0x23,0x20,0x30,0x00},/*"Z",58*/

{0x00,0x00,0xFE,0x02,0x02,0x00,0x00,0x00,0x7F,0x40,0x40,0x00},/*"[",59*/

{0x00,0x38,0xC0,0x00,0x00,0x00,0x00,0x00,0x01,0x1E,0x60,0x00},/*"\",60*/

{0x00,0x02,0x02,0xFE,0x00,0x00,0x00,0x40,0x40,0x7F,0x00,0x00},/*"]",61*/

{0x00,0x04,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"^",62*/

{0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80},/*"_",63*/

{0x00,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"`",64*/

{0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x3D,0x26,0x22,0x3F,0x20},/*"a",65*/

{0x08,0xF8,0x00,0x80,0x00,0x00,0x00,0x1F,0x21,0x20,0x1F,0x00},/*"b",66*/

{0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x1F,0x21,0x20,0x21,0x00},/*"c",67*/

{0x00,0x00,0x80,0x88,0xF8,0x00,0x00,0x1F,0x20,0x20,0x3F,0x20},/*"d",68*/

{0x00,0x00,0x80,0x80,0x00,0x00,0x00,0x1F,0x22,0x22,0x23,0x00},/*"e",69*/

{0x00,0x80,0xF0,0x88,0x88,0x18,0x00,0x20,0x3F,0x20,0x20,0x00},/*"f",70*/

{0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x6B,0x94,0x94,0x93,0x60},/*"g",71*/

{0x08,0xF8,0x00,0x80,0x80,0x00,0x20,0x3F,0x21,0x00,0x3F,0x20},/*"h",72*/

{0x00,0x80,0x98,0x00,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00},/*"i",73*/

{0x00,0x00,0x80,0x98,0x00,0x00,0xC0,0x80,0x80,0x7F,0x00,0x00},/*"j",74*/

{0x08,0xF8,0x00,0x80,0x80,0x80,0x20,0x3F,0x24,0x06,0x39,0x20},/*"k",75*/

{0x08,0x08,0xF8,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00},/*"l",76*/

{0x80,0x80,0x80,0x80,0x00,0x00,0x3F,0x00,0x3F,0x00,0x3F,0x00},/*"m",77*/

{0x80,0x80,0x00,0x80,0x80,0x00,0x20,0x3F,0x21,0x00,0x3F,0x20},/*"n",78*/

{0x00,0x00,0x80,0x80,0x00,0x00,0x00,0x1F,0x20,0x20,0x1F,0x00},/*"o",79*/

{0x80,0x80,0x80,0x80,0x00,0x00,0x80,0xFF,0xA0,0x20,0x1F,0x00},/*"p",80*/

{0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x1F,0x20,0xA0,0xFF,0x80},/*"q",81*/

{0x80,0x80,0x00,0x80,0x80,0x00,0x20,0x3F,0x21,0x00,0x01,0x00},/*"r",82*/

{0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x23,0x24,0x24,0x39,0x00},/*"s",83*/

{0x00,0x80,0xE0,0x80,0x00,0x00,0x00,0x00,0x1F,0x20,0x20,0x00},/*"t",84*/

{0x80,0x80,0x00,0x80,0x80,0x00,0x00,0x3F,0x20,0x20,0x3F,0x20},/*"u",85*/

{0x80,0x80,0x00,0x00,0x80,0x80,0x00,0x07,0x38,0x1C,0x03,0x00},/*"v",86*/

{0x80,0x40,0x80,0x00,0x80,0x00,0x03,0x3C,0x03,0x3E,0x03,0x00},/*"w",87*/

{0x80,0x80,0x00,0x80,0x80,0x00,0x20,0x33,0x0E,0x3B,0x20,0x00},/*"x",88*/

{0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x87,0x78,0x0C,0x03,0x00},/*"y",89*/

{0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x30,0x2C,0x23,0x30,0x00},/*"z",90*/

{0x00,0x00,0x80,0x7E,0x02,0x00,0x00,0x00,0x00,0x3F,0x40,0x00},/*"{",91*/

//{0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00},/*"|",92*/
{0x1C,0x22,0x1C,0xC0,0x40,0x40,0x00,0x00,0x00,0x3F,0x02,0x02},/*"未命名文件",DEG AND F*/

//{0x00,0x02,0x7E,0x80,0x00,0x00,0x00,0x40,0x3F,0x00,0x00,0x00},/*"}",93*/
{0xF8,0xF0,0xE0,0xC0,0x80,0x00,0x0F,0x07,0x03,0x01,0x00,0x00},/*"未命名文件",0 */

//{0x06,0x01,0x02,0x04,0x04,0x03,0x00,0x00,0x00,0x00,0x00,0x00},/*"~",94*/ 
{0x1C,0x22,0x1C,0x80,0x40,0x40,0x00,0x00,0x00,0x1F,0x20,0x20},/*"未命名文件",DEG AND C */

{0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00},/*"|",95*/



};


void Lcd_Write_Byte(unsigned char num) //from high to low
{
	char data i;
	LCD_CS = 0;
	_nop_();
	LCD_A0 = 1;
	_nop_();	
	for(i = 0;i < 8;i++)
	{
		if(num & 0x80)	LCD_DATA = 1;
     	else       	LCD_DATA = 0;
		
		_nop_();	
     	LCD_CLK = 0;
		_nop_();
		LCD_CLK = 1;
		_nop_();

     	num <<= 1;	
	}
	_nop_();
	LCD_CS = 1;	 _nop_();
	LCD_DATA = 1;
	_nop_();
}


void Lcd_Write_Command(unsigned char command) //from high to low
{
	char data i;
	LCD_CS = 0;
	_nop_();
	LCD_A0 = 0;
	_nop_();	
	for(i = 0;i < 8;i++)
	{
		if(command & 0x80)	LCD_DATA = 1;
     		else       		LCD_DATA = 0;
			
		_nop_();
     	LCD_CLK = 0;
		_nop_();

		LCD_CLK = 1;
		_nop_();

     	command <<= 1;
	}
	_nop_();
	LCD_CS = 1;
	LCD_DATA = 1;_nop_();	
	_nop_();
}



void Lcd_Write_Char(unsigned char row,unsigned char line,unsigned char num,char mode) // row: 0 - 4 , line: 0 - 21
{

	unsigned char data loop; 
	unsigned char data index = num - ' ';
	
	Lcd_Write_Command(0x07);	
	Lcd_Set_X_Addr((line + 1) * 6);
	Lcd_Set_Y_Addr(row * 2);
	for(loop = 0;loop < 6;loop++) 
		if(mode == 1)	Lcd_Write_Byte(nAsciiDot[index][0][loop]);  // Normal
		else  Lcd_Write_Byte(~nAsciiDot[index][0][loop]);			// inverse
	Lcd_Set_X_Addr((line + 1) * 6);
	Lcd_Set_Y_Addr(row * 2+1);
	for(loop = 0;loop < 6;loop++) 
		if(mode == 1)	Lcd_Write_Byte(nAsciiDot[index][1][loop]);  // Normal
		else Lcd_Write_Byte(~nAsciiDot[index][1][loop]);			// inverse
	Lcd_Write_Command(0x06);

}

void Lcd_Set_Fuction(char mode)
{
	unsigned char fuction;
	fuction = 0x38 + mode;//0x20 + mode;
	Lcd_Write_Command(fuction);
}

void Lcd_Set_Y_Addr(unsigned char page)// 0 1 0 0        page 0 - 9
{
	unsigned char addr;
	Lcd_Set_Fuction(0);
	addr = 0x40 + page;
	Lcd_Write_Command(addr);
}

void Lcd_Set_X_Addr(unsigned char line)// 1 1 1 0/1        page 0 - 129
{
	unsigned char addr;
	unsigned char line_low;
	unsigned char line_high;
	Lcd_Set_Fuction(0);
	line_low = line & 0x0f;
	line = line >> 4;
	line_high = line; 
	addr = 0xe0 + line_low; //low
	Lcd_Write_Command(addr);
	addr = 0xf0 + line_high; //high
	Lcd_Write_Command(addr);
}


void Lcd_Initial(void)
{
	LCD_RESET = 0;
	DELAY_Us(1000);
	LCD_RESET = 1;
	DELAY_Us(100);

	
	Lcd_Set_Fuction(1); // 0x21		 
	//DELAY_Us(10);
	Lcd_Write_Command(CMD_SET_BIAS);  //DELAY_Us(10);
	Lcd_Write_Command(CMD_SET_V0);	 //DELAY_Us(10);
	Lcd_Write_Command(CMD_DISPLAY_CONFIG); //DELAY_Us(10);
	
	Lcd_Set_Fuction(0); //DELAY_Us(10); // 0x20
	Lcd_Write_Command(CMD_SET_V0Range_HIGH);  //DELAY_Us(10);  
	Lcd_Write_Command(CMD_DISPLAT_NORMAL); //DELAY_Us(10);
	Lcd_Set_X_Addr(0); //DELAY_Us(10); //0xf0 0xe0
	Lcd_Set_Y_Addr(0); //DELAY_Us(10); //0x40	

	Lcd_All_Off();
	
}

void Lcd_All_Off(void)
{
	unsigned int i;
	Lcd_Write_Command(0x07);
	for(i = 0;i < 130*80;i++)	
	{
		Lcd_Write_Byte(0x00);
	}
	Lcd_Write_Command(0x06);
}

void Lcd_Show_String(U8_T pos_x, U8_T pos_y, S8_T* str,U8_T mode,U8_T format)
{
 	U8_T loop;	
	U8_T len = format & 0x7f; /* the higest bit is for selecting capital or uncial, the real lenght is "format & 0x7f" */
	U8_T str_len;
	U8_T real_len;
	S8_T far tempstr[21];
	if(len > 21)	len = 21;
	if(strlen(str) > 21) str_len = 21;
	else	str_len = strlen(str);
	if(strlen(str) > 21)   real_len = 14;
	else
		real_len = strlen(str);
	for(loop = 0;loop < real_len;loop++)
	{ /* check the higheset bit, it is for capital or uncial */
		if(format & 0x80)	tempstr[loop] = (S8_T)toupper(str[loop]); /* capital */
		else tempstr[loop] = str[loop]; /* uncial */
	}

	/* if the real len is bigger than the strings, using space to fill the residual area*/
	if(len > real_len)
	{	
			/* fill space */
		for(loop = strlen(str);loop < len;loop++)
		{
			Lcd_Write_Char(pos_x,pos_y + loop,' ',mode);
		}	
			/* copy read data to LCD buffer */
		for(loop = 0;loop < strlen(str);loop++)
		{
			Lcd_Write_Char(pos_x,pos_y + loop,tempstr[loop],mode);		
		}
	}
	/* if the real len is less than the string, cut the string */
	/* if len is 0, using string len */
	else
	{	
		if(len == 0)	len = real_len;	
		for(loop = 0;loop < len;loop++)
		{
			Lcd_Write_Char(pos_x,pos_y + loop,tempstr[loop],mode);
		}
	}
}



void Lcd_Show_Data(char pos_x,char pos_y,unsigned int number,char dot,char mode) //,char div, bool signed)
{
	char  far loop = 0;
	char far num[5];
	char  far length;
	char far  by_x,by_y;

	by_x = pos_x;
	by_y = pos_y;	

	//number = number / div;

	if(number >= 1000)	length = 4;
	else if(number >= 100)	length = 3;
	else if(number >= 10)	length = 2;
	else length = 1;

	if((length > 1) && (dot != 0)) 
	{
		length ++;	
		num[dot] = '.';
		num[4] = number / 1000; 	number = number % 1000;
		switch(dot)
		{
			case 3:
					num[2] = number / 100; 		number = number % 100;
					num[1] = number / 10; 		number = number % 10;
					num[0] = number;
					break;
			case 2:
					num[3] = number / 100; 		number = number % 100;
					num[1] = number / 10; 		number = number % 10;
					num[0] = number;
					break;
			case 1:
					num[3] = number / 100; 		number = number % 100;
					num[2] = number / 10; 		number = number % 10;
					num[0] = number;
					break;
		}

	}
	else
	{
		num[3] = number / 1000; 	number = number % 1000;
		num[2] = number / 100; 		number = number % 100;
		num[1] = number / 10; 		number = number % 10;
		num[0] = number;
	}	

//	Lcd_Write_Command(0x07);	
	for(loop = length - 1;loop >= 0;loop--)
	{
		if(num[loop] == '.')	Lcd_Write_Char(by_x,by_y,'.',mode);
		else
		Lcd_Write_Char(by_x,by_y,aby_Char_Table[num[loop]],mode);
		by_y ++;
		if(by_y >= 21)
		{
			by_y = 0;
			by_x ++;
			if(by_x >= 5)	by_x = 0;
		}
	}
	if(length < 4)
	for(loop = 4;loop >= length;loop--)
	{
		 Lcd_Write_Char(by_x,by_y - length + loop,' ',mode);
	}	
//	Lcd_Write_Command(0x06);
		
}

char far scroll_message[200];
char far scroll_message_length;

bit scrolling_flag = TRUE;

void start_scrolling(void)
{
	scrolling_flag = TRUE;
}

void stop_scrolling(void)
{
	scrolling_flag = FALSE;
}

char time[] = "2012-09-19 17:16:10";
void get_time_text(void)
{

	time[2] =  RTC.Clk.year / 10 + '0';
	time[3] =  RTC.Clk.year % 10 + '0';
	time[4] = '-';
	time[5] =  RTC.Clk.mon / 10 + '0';
	time[6] =  RTC.Clk.mon % 10 + '0';
	time[7] = '-';
	time[8] =  RTC.Clk.day / 10 + '0';
	time[9] =  RTC.Clk.day % 10 + '0';
	time[11] =  RTC.Clk.hour / 10 + '0';
	time[12] =  RTC.Clk.hour % 10 + '0';
	time[14] =  RTC.Clk.min / 10 + '0';
	time[15] =  RTC.Clk.min % 10 + '0';
	time[17] =  RTC.Clk.sec / 10 + '0';
	time[18] =  RTC.Clk.sec % 10 + '0';

}

U8_T const code network_status_text[] = "  NETWORK:";
U8_T const code main_net_status_text[] = " Main net:";
U8_T const code sub_net_status_text[] = " Subnet:";
U8_T const code net_status_ok_text[] = "OK";
U8_T const code net_status_dead_text[] = "Dead";
U8_T const code net_offline_text[] = "Offline";
U8_T const code alarm_text[] = "  ALARM:";

void update_message_context(void)
{
	U8_T data length;

	if(scrolling_flag == 1)
	{	  
	scroll_message_length = 0;

	length = sizeof(time) - 1;
	get_time_text();
	memcpy(scroll_message + scroll_message_length, time, length);
	scroll_message_length += length;

// Network
	length = sizeof(network_status_text) - 1;
	memcpy(scroll_message + scroll_message_length, network_status_text, length);
	scroll_message_length += length;
	// main net status
	length = sizeof(main_net_status_text) - 1;
	memcpy(scroll_message + scroll_message_length, main_net_status_text, length);
	scroll_message_length += length;
	//if(main_net_status_ctr)
	{
		length = sizeof(net_status_ok_text) - 1;
		memcpy(scroll_message + scroll_message_length, net_status_ok_text, length);
		scroll_message_length += length;
	}
	/*else
	{
		length = sizeof(net_status_dead_text) - 1;
		memcpy(message + scroll_message_length, net_status_dead_text, length);
		scroll_message_length += length;
	}*/
	// sub net status
	length = sizeof(sub_net_status_text) - 1;
	memcpy(scroll_message + scroll_message_length, sub_net_status_text, length);
	scroll_message_length += length;
	//if(db_ctr == current_online_ctr)
	{
		length = sizeof(net_status_ok_text) - 1;
		memcpy(scroll_message + scroll_message_length, net_status_ok_text, length);
		scroll_message_length += length;
	}
	/*else
	{
		U8_T data i, id_temp;
		for(i = 1; i < db_ctr; i++)
		{
			id_temp = read_eeprom(SCAN_DB_START_ADDR + 5*i);
			if((current_online[id_temp / 8] & (1 << (id_temp % 8))) == 0x00)
			{
				strcpy(text, external_text);
				itoa(id_temp, int_text);
//				sprintf(int_text, "%u", id_temp);
				strcat(text, int_text);
				strcat(text, " ");

				length = strlen(text);
				memcpy(message + scroll_message_length, text, length);
				scroll_message_length += length;
			}
		}

		length = sizeof(net_offline_text) - 1;
		memcpy(message + scroll_message_length, net_offline_text, length);
		scroll_message_length += length;
	}  */

// Alarm
	length = sizeof(alarm_text) - 1;
	memcpy(scroll_message + scroll_message_length, alarm_text, length);
	scroll_message_length += length;


// SPACE
	length = 3;
	memcpy(scroll_message + scroll_message_length, "   ", length);
	scroll_message_length += length;
	}

}


void display_character_with_start_bit(U8_T row, U8_T start_line, U8_T start_bit, U8_T end_bit, U8_T c, U8_T disp_mode)
{
	U8_T data i;
	U8_T data index = c - ' ';

	if(start_line >= 130)
		return;
	Lcd_Write_Command(0x07);	
	Lcd_Set_X_Addr(start_line);
	Lcd_Set_Y_Addr(row * 2);
	for(i = start_bit; i < end_bit; i++)
	{ 
		if(disp_mode == NORMAL)
			Lcd_Write_Byte(Dot8_16[index][0][i]);
		else
			Lcd_Write_Byte(~Dot8_16[index][0][i]);
	}

	
	Lcd_Set_X_Addr(start_line);
	Lcd_Set_Y_Addr(row * 2 + 1);
	for(i = start_bit; i < end_bit; i++)
	{ 
		if(disp_mode == NORMAL)
			Lcd_Write_Byte(Dot8_16[index][1][i]);
		else
			Lcd_Write_Byte(~Dot8_16[index][1][i]);
	}
	Lcd_Write_Command(0x06);
}

void scrolling_message(void)
{
	char data i, disp_value, byte_index;
	static char start_bit = 0;
	static char start_byte = 0;

	if(scrolling_flag != FALSE)
	{//	return;

	byte_index = start_byte;

	disp_value = scroll_message[byte_index++];
	byte_index %= scroll_message_length;
	display_character_with_start_bit(4, 0, start_bit, 8, disp_value, NORMAL);
	if(start_bit < 7)
	{
		for(i = 1; i < 16; i++)
		{
			disp_value = scroll_message[byte_index++];
			byte_index %= scroll_message_length;
			display_character_with_start_bit(4, 8 * i - start_bit, 0, 8, disp_value, NORMAL);
		}
	
		disp_value = scroll_message[byte_index++];
		byte_index %= scroll_message_length;
		display_character_with_start_bit(4, 8 * i - start_bit, 0, 4 + start_bit, disp_value, NORMAL);
	}
	else // if(start_bit == 7)
	{
		for(i = 1; i < 17; i++)
		{
			disp_value = scroll_message[byte_index++];
			byte_index %= scroll_message_length;
			display_character_with_start_bit(4, 8 * i - start_bit, 0, 8, disp_value, NORMAL);
		}
	
		disp_value = scroll_message[byte_index++];
		byte_index %= scroll_message_length;
		display_character_with_start_bit(4, 8 * i - start_bit, 0, start_bit - 2, disp_value, NORMAL);
	} 

	start_bit++;
	if(start_bit >= 8)
	{
		start_bit = 0;
		start_byte++;
		start_byte %= scroll_message_length;
	}
	}
}
