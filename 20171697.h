#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define TRUE 1
#define FALSE 0
//Define constant number
#define command_num 13
#define MAX_PATH 260
#define MAX_COMMAND 500
#define MEM_SIZE 1048576
#define MEM_LIMIT 1048575
#define HASH_TABLE_SIZE 20
#define SYMBOL_TABLE_SIZE 20

enum Command{
	Help, Dir, Quit, History, Dump, Edit, Fill, Reset, Opcode, Opcodelist, Assemble, Type, Symbol, Etc
};
enum Mode{
	Store_input, Erase_space
};
enum CharType{
	ENTER=1, COMMA, CHAR, BLANK
};

//  Structure for storing past command
typedef struct input{
	char  str[MAX_COMMAND];
	struct input *next;
}input;

//  Structure for storing (mnemonic,opcode)
typedef struct opcode_info{
	char mnemonics[7];
	unsigned char opcode;
	int format[4];
	struct opcode_info *next;
}opcode_info;

//  Structure for symbol
typedef struct symbol_info{
	char name[7];
	unsigned int address;
	struct symbol_info *next;
}symbol_info;

// Structure for error-list
typedef struct error{
	int line;
	char message[100];
	struct error *next;
}

void Init();
int Hash_func(char mnemonics[]);
void Make_hash_table();
int Get_Command();

//Execute command
void help(); 
void show_files(); //dir
void quit();
void show_history(); //history
void mem_dump();
void mem_edit();
void mem_fill();
void mem_reset();
void opcode();
void opcodelist();
void show_content();
void assemble();
int assem_pass1( char filename[] );
void write_lst( char filename[], unsigned int LOCCTR, int arg_num, char comment[], char symbol[],  char mnemonic[], char operand[]);
int read_one_line(FILE *fp, char comment[], char symbol[],  char mnemonic[], char operand[]);
void push_symbol( char symbol[], unsigned int LOCCTR );
int find_opcode( char mne[] );
void assem_pass2();

//Manage input string
int Str_convert_into_Hex( char str[], unsigned int *num);
void Hex_convert_into_Str( unsigned int num, int len);
int Handling_Input( int mode, char input_str[], char str[], int len, int HEXA);
int Get_argument( unsigned int *arg1, unsigned int *arg2, unsigned int *arg3, int arg_len[]);
int Get_String_Argument( char filename[]);
void Write_Hex(FILE *fp, unsigned int num, int len);
void Write_Blank(FILE *fp, int len);

void store_input();
void erase_symtab();
void end_program();
