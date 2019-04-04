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
enum MODE{
	Store_input, Erase_space
};
enum CHARTYPE{
	ENTER = 1, COMMA, CHAR, BLANK
};
enum DIRECTIVE{
	START=1, END, BYTE, WORD, RESB, RESW, BASE, NOBASE
};
enum Instruction{
	Nothing = -1, Directive, Format1, Format2, Format3, Format4
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
	int format[5];
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
}error;

// Structure for assembler
typedef struct assemble_flags{
	int start;
	int end;
	int error;
	int base;
	char OutputFileName[100];
	char OutputFileName2[100];

}assemble_flags;

// Structure for Object code Writer
typedef struct object_program{
	char code[70];
	int current_col;
	unsigned int modify_record[500];
}object_program;


void Init();
int Hash_func(char mnemonics[]);
void Make_hash_table();
int Get_Command();

// Execute command
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

// Assemble
void assem_pass1( char filename[] );
void show_error_list();
void write_interm( unsigned int LOCCTR, int arg_num, char comment[], char *symbol,  char *mnemonic, int inst_flag, int num, char *operand);
int Is_Directive ( char *instruction , char *operand, unsigned int *inc, int line_num , int *dir_num );
int Is_Mnemonic( char *mnemonic , char *operand, unsigned int *inc, int line_num, int*opcode);
int push_symbol( char symbol[], unsigned int LOCCTR );
void push_into_error_list( error *cur);
int find_opcode( char mne[], int *opcode);
void assem_pass2();


// Manage input string
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
