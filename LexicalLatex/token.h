#pragma once
#define NONBR 0
#define RNDBR 1
#define SQUBR 2
#define SWGBR 3
#define ABSBR 4

#define TYPBRAC 1 //Bracket
#define TYPWORD 2 //Word from wordlist
#define TYPLINE 3 //New Line
#define TYPTOKN 4 //Token sign f.E. +-/*
#define TYPLCMD 5 //Latex Command
class token
{
private:
	int type = 0;
	int hierarchy = 0;
	int lasthierarchytype =NONBR;
	int wordlistid = 0;
public:
	token(int p_type, int p_hierarchy, int p_lasthierarchytype);
	token(int p_type, int p_hierarchy, int p_lasthierarchytype, int p_wordlistid);
	int get_type();
	int get_hierarchy();
	int get_lasthierarchytype();
	int get_wordlistid();
	void set_type(int _type);
	void set_hierarchy(int _hierarchy);
	void set_lasthierarchytype(int _lasthierarchytype);
	void set_wordlistid(int _wordlistid);
	void become_SWGBR_Left();
	void become_SWGBR_Right();
	~token();
};