//  Linked List for past command - Queue
input *History_Head;
input *History_Tail;
input *new_input;


//  index that indicates where to start to read input text
int rd_pt;


//  Store command list
char* command_list[command_num][2];


//  1MB Memory space
unsigned char *Memory;
int last_mem_idx; //index that indicates where ends printing

//  Hash table
opcode_info* Hash_Table[HASH_TABLE_SIZE];


// Symtab
symbol_info* SYMTAB[SYMBOL_TABLE_SIZE];


// Error List
error *Error_list_head;
error *Error_list_tail;


// For assembler
char *reg[reg_num];
Assemble_Info ASBL;
object_program OBJ;


// Flag
int Exit_flag;
int Success;
