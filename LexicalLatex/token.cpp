#include "stdafx.h"
#include "token.h"


token::token(int p_type, int p_hierarchy, int p_lasthierarchytype)
{
	type = p_type;
	wordlistid = 0;
	hierarchy = p_hierarchy;
	lasthierarchytype = p_lasthierarchytype;
}

token::token(int p_type, int p_hierarchy, int p_lasthierarchytype, int p_wordlistid)
{
	type = p_type;
	wordlistid = 0;
	hierarchy = p_hierarchy;
	lasthierarchytype = p_lasthierarchytype;
	wordlistid = p_wordlistid;
}

int token::get_type() {
	return type;
}
int token::get_hierarchy() {
	return hierarchy;
}
int token::get_lasthierarchytype() {
	return lasthierarchytype;
}
int token::get_wordlistid() {
	return wordlistid;
}

void token::set_type(int _type) {
	type = _type;
}
void token::set_hierarchy(int _hierarchy) {
	hierarchy = _hierarchy;
}
void token::set_lasthierarchytype(int _lasthierarchytype) {
	lasthierarchytype = _lasthierarchytype;
}
void token::set_wordlistid(int _wordlistid) {
	wordlistid = _wordlistid;
}
void token::become_SWGBR_Left() {
	lasthierarchytype=TYPBRAC;
	type=TYPLCMD;
	wordlistid=1;
}
void token::become_SWGBR_Right() {
	lasthierarchytype = TYPBRAC;
	type = TYPLCMD;
	wordlistid = 2;
}

token::~token()
{
}
