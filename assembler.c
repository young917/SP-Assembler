#include "20171697.h"
#include "variable.h"

void assemble(){
	char filename[MAX_COMMAND];
	int type, arg_num;
	int error_flag;

	type = Get_String_Argument( filename );
	if( Success == FALSE || type != ENTER){
		Success = FALSE;
		return;
	}
	error_flag = assem_pass1(filename);
	if( error_flag == TRUE ){
		show_error_list();
		// remove lst file
		Success = FALSE;
		return;
	}
	error_flag = assem_pass2(filename);
	if( error_flag == TRUE ){
		show_error_list();
		// remove lst file and obj file
		Success = FALSE;
		return;
	}
}

void erase_symtab(){
	int idx;
	symbol_info *present;
	symbol_info *next;

	for( idx = 0; idx < SYMBOL_TABLE_SIZE ; idx++ ){
		present = next = SYMTAB[idx];
		while( next != NULL ){
			present = next;
			next = present->next;
			free(present);
			present = NULL;
		}
	}
}
void show_error_list(){
	error *cur, *pre;

	pre = cur = Error_list_head;
	while( cur != NULL ){
		pre = cur;
		cur = pre->next;
		printf("line	%d	:	%s\n", pre->line, pre->message);
		free(pre);
		pre = NULL;
	}

}

int assem_pass1( char filename[] ){
	FILE *fp;
	char symbol[7];
	char mnemonic[7];
	char operand[10];
	char comment[200];
	unsigned int LOCCTR = 0;
	char *start = "START";
	char *byte = "BYTE";
	char *word = "WORD";
	char *resb = "RESB";
	char *resw = "RESW";
	char *end = "END";
	unsigned int value;
	int format;
	int len;
	int arg_num;
	int ret;
	int end_flag;
	error *new_error;
	int error_flag;
	int line_num = 5;

	fp = fopen(filename,"r");
	if( fp == NULL ){
		Success = FALSE;
		return 0;
	}

	erase_symtab();

	while(TRUE){
		arg_num = read_one_line(fp, comment, symbol, mnemonic, operand);
		if( !error_flag )
			write_lst(filename, LOCCTR, arg_num, comment, symbol, mnemonic, operand);

		if( arg_num > 3 ){// Error
			
			new_error = (error*)malloc(sizeof(error));
			new_error->line = line_num;
			strcpy( new_error->message, "Improper number of columns");
			new_error->next = NULL;
			push_into_error_list( new_error );
			error_flag = TRUE;
			line_num += 5;
			continue;
		}

		else if( arg_num == 0 )
			break;

		else if( strcmp(mnemonic,start) == 0 ){// START

			ret = Str_convert_into_Hex(operand, &value);
			if( ret == FALSE){
				new_error = (error*)malloc(sizeof(error));
				new_error->line = line_num;
				strcpy( new_error->message, "operand has wrong character.");
				new_error->next = NULL;
				push_into_error_list( new_error );		
				error_flag = TRUE;
			}
			else
				LOCCTR = value;
			line_num += 5;
			continue;
		}

		if( arg_num == 3 ){//  Make SYMTAB
			ret = push_symbol(symbol, LOCCTR);
			if( ret == FALSE ){
				new_error = (error*)malloc(sizeof(error));
				new_error->line = line_num;
				strcpy( new_error->message, "fail to make SYMTAB.");
				new_error->next = NULL;
				push_into_error_list( new_error );		
				error_flag = TRUE;
				continue;
			}
		}
		if( arg_num != 1 ){// Judge proper mnemonic

			format = find_opcode( mnemonic );
			if( format != 0){
				switch( format ){
					case 1: LOCCTR += 1; break;
					case 2: LOCCTR += 2; break;
					case 3: {
								if( operand[0] == '+' )
									LOCCTR += 4;
								else
									LOCCTR += 3;
								break;
							}
				}
			}
			else if( strcmp( mnemonic, end) == 0 ){
				end_flag = TRUE;
			}
			else if( strcmp( mnemonic, byte) == 0 ){
				len = strlen( operand );
				if( operand[1] != ''' || operand[len-1] != ''')
					error_flag = TRUE;
				if( operand[0] == 'C'){
					len -= 3;
					LOCCTR += len;
				}
				else if( operand[0] == 'X'){
					len -= 3;
					len = ( len + 1 ) / 2;
					LOCCTR += len;
				}
				// I don't know how to deal with decimal...?
			}
			else if( strcmp( mnemonic, word ) == 0 ){
				LOCCTR += 3;
			}
			else{
				if( operand[0] == '+' || operand[0] == '#' || operand[0] == '@' ){
					strcpy( operand, operand + 1 );
					ret = Str_convert_into_Hex( operand, &value );
				}
				ret = Str_convert_into_Hex( operand, &value );
				if( ret == FALSE){
					new_error = (error*)malloc(sizeof(error));
					new_error->line = line_num;
					strcpy( new_error->message, "operand has wrong character.");
					new_error->next = NULL;
					push_into_error_list( new_error );
					error_flag = TRUE;
					continue;
				}
				if( strcmp( mnemonic, resb) == 0 ){
					LOCCTR += value;
				}
				else if( strcmp( mnemonic, resw) == 0 ){
					LOCCTR += ( 3 * value );
				}
				else{
					new_error = (error*)malloc(sizeof(error));
					new_error->line = line_num;
					strcpy( new_error->message, "operand has wrong character.");
					new_error->next = NULL;
					push_into_error_list( new_error );
					error_flag = TRUE;			
				}
			}
		} 
		line_num += 5;
	}

	if(!end_flag){//Error
		error_flag = TRUE;
		new_error = (error*)malloc(sizeof(error));
		new_error->line = line_num;
		strcpy( new_error->message, "not proper end of file.");
		new_error->next = NULL;
		push_into_error_list( new_error );		
		error_flag = TRUE;
	}
	
	fclose(fp);
	return error_flag;
}

void assem_pass2(){
	
}

void write_lst( char filename[], unsigned int LOCCTR, int arg_num, char comment[], char symbol[],  char mnemonic[], char operand[]){
	
	FILE *fp;
	char lst_name[100];
	char *extend = ".lst";
	char flag;
	int len;

	strcpy(lst_name, filename);
	strcat(lst_name, extend);

	fp = fopen(lst_name, "a");
	Write_Hex(fp, LOCCTR, 4);
	if( arg_num == 1){
		fprintf(fp, "%s", comment);
		return;
	}
	else if( arg_num < 1 || arg_num > 3 )
		return;
	
	Write_Blank(fp, 4);
	fprintf(fp, "%s", symbol);
	len = 10 - strlen(symbol);
	Write_Blank(fp, len);
	fprintf(fp, "%s", mnemonic);
	len = 9 - strlen(mnemonic);
	Write_Blank(fp, len);
	flag = operand[0];
	if(flag == '#' || flag == '@' || flag == '+')
		fprintf(fp, "%s\n", operand);
	else
		fprintf(fp, " %s\n", operand);
	fclose(fp);
}

int read_one_line(FILE *fp, char comment[], char symbol[],  char mnemonic[], char operand[]){
	char line[200];
	char *pt;
	char *tmp[4];
	int arg_num = 0;

	pt = fgets(line, 200, fp);
	if(pt == NULL)
		return 0;

	strcpy( comment, line);

	tmp[arg_num] = strtok( line, "\t\n ");
	while( tmp[arg_num] != NULL ){
		arg_num++;
		if( arg_num >3 )
			break;
		tmp[arg_num] = strtok(NULL,"\t\n ");
	}

	if( tmp[0][0] == '.' ){// Case: Comment
		mnemonic[0] = '.';
		return 1;
	}
	else if( arg_num == 1)// error
		return 4;


	switch( arg_num ){
		case 2:{
				   strcpy(mnemonic, tmp[0]);
				   strcpy(operand, tmp[1]);
				   break;
			   }
		case 3:{
				   strcpy(symbol, tmp[0]);
				   strcpy(mnemonic, tmp[1]);
				   strcpy(operand, tmp[2]);
			   }
	}
	return arg_num;
}

int push_symbol( char symbol[], unsigned int LOCCTR ){

	int ret = TRUE;
	symbol_info *new_sym;
	symbol_info *pre, *cur;
	int Hash_value;

	new_sym = (symbol_info *)malloc(sizeof(symbol_info));
	if( new_sym == NULL ){
		return FALSE;
	}

	strcpy( new_sym->name, symbol );
	new_sym->address = LOCCTR;
	Hash_value = Hash_func( symbol );

	pre = NULL;
	cur = SYMTAB[Hash_value];

	while( cur != NULL ){
		switch( strcmp(symbol, cur->name) ){
			case 0: ret = FALSE; break;
			case 1: break;
		}
		pre = cur;
		cur = cur->next;
	}
	if( ret == FALSE )
		return ret;

	if( pre == NULL ){
		new_sym->next = SYMTAB[Hash_value];
		SYMTAB[Hash_value] = new_sym;
	}
	else{
		new_sym->next = cur;
		pre->next = new_sym;
	}
	return ret;
}

void push_into_error_list(error *cur){
	if( Error_list_head == NULL ){
		Error_list_head = cur;
		Error_list_tail = cur;
	}
	else{
		Error_list_tail->next = cur;
		Error_list_tail = cur;
	}
}

int find_opcode( char mnemonic[] ){
	int adr;
	int format;

	adr = Hash_func(mnemonic);
	cur = Hash_Table[adr];
	while(cur != NULL){
		if( strcmp(cur->mnemonics, mnemonic) == 0)
			break;
		cur = cur->next;
	}

	//Not found
	if( cur == NULL){
		return FALSE;
	}

	for( format = 0; format < 4; format++ )
		if( cur->format[format] == 1 )
			break;
	return format;
}
