#include "20171697.h"
#include "variable.h"

void assemble(){
	char filename[100];
	char lstname[100];
	int type, arg_num, len;
	int code_len;


	type = Get_String_Argument( filename );
	if( Success == FALSE || type != ENTER){
		Success = FALSE;
		return;
	}

	// Initailize
	Assemble_State.start = FALSE;
	Assemble_State.end = FALSE;
	Assemble_State.error = FALSE;
	Assemble_State.base = FALSE;
	erase_symtab();

	code_len = assem_pass1(filename);
	
	if( Assemble_State.error == TRUE ){
		show_error_list();
		
		remove(Assemble_State.OutputFileName);

		erase_symtab();
		Success = FALSE;
		return;
	}

	assem_pass2(filename, code_len );
	
	if( Assemble_State.error == TRUE ){
		show_error_list();

		remove(Assemble_State.OutputFileName);

		len = strlen( filename );
		strncpy(lstname, filename, len-4);
		lstname[len-4] = '\0';
		strcat(lstname, ".lst");
		remove(lstname);

		erase_symtab();
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

void show_error_list(){// And erase
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

void assem_pass1( char filename[] ){
	//return error_flag

	FILE *fp;
	FILE *fp2;
	char lst_name[100];
	char *extend = ".lst";

	char *token[3];
	char line[200];
	char comment[200];
	char *blank = "\0";
	int arg_num;
	char *pt;
	char *tmp;

	unsigned int value = 0;
	unsigned int LOCCTR = 0;

	int len, ret;
	int inst_num;
	int line_num = 5;
	int i;
	error *new_error;

	fp = fopen(filename,"r");
	if( fp == NULL ){
		Success = FALSE;
		Assemble_State.error = TRUE;
		return;
	}

	strcpy( Assemble_State.OutputFileName, filename );
	strcat( Assemble_State.OutputFileName, ".txt" );
	fp2 = fopen( Assemble_State.OutputFileName, "w" );
	fclose( fp2 );

	while(TRUE){

		arg_num = 0;

		pt = fgets(line, 200, fp);
		if(pt == NULL)// FILE END
			break;

		comment[0] = '\0';
		strcpy( comment, line );

		token[arg_num] = strtok( line, "\t\n ");
		while ( token[arg_num] != NULL ){

			// make operand ' A  ,  B ' -> ' A,B '
			if ( arg_num > 1 && token[arg_num][0] == ',' ){
				strcat( token[arg_num-1], token[arg_num] );
				arg_num--;
			}
			len = strlen( token[arg_num] );
			if( len >= 2 && token[arg_num][len-1] == ','){
				tmp = strtok( NULL, "\t\n ");
				if( tmp != NULL )
					strcat( token[arg_num], tmp);
			}

			arg_num++;
			if( arg_num == 3 )
				break;
			token[arg_num] = strtok( NULL, "\t\n ");
		}

		if( token[0] != NULL && token[0][0] == '.' ){// COMMENT
			write_interm( line_num, LOCCTR, 0, comment, blank, blank, FALSE, 0,blank );
			line_num += 5;
			continue;
		}
		
		else if( arg_num == 0 ){// BLANK
			line_num += 5;
			continue;
		}

		else if( Assemble_State.end == TRUE ){
			new_error = (error*)malloc(sizeof(error));
			new_error->line = line_num;
			strcpy( new_error->message, "After END, There exist more code.");
			new_error->next = NULL;				
			push_into_error_list( new_error );		
			Assemble_State.error = TRUE;
			break;
		}

		tmp = strtok( NULL, "\t\n ");
		if( (tmp != NULL) ){// Excess : token more than 3
			new_error = (error*)malloc(sizeof(error));
			new_error->line = line_num;
			strcpy( new_error->message, "Improper number of columns");
			new_error->next = NULL;
			push_into_error_list( new_error );
			Assemble_State.error = TRUE;
			line_num += 5;
			continue;
		}

		ret = Is_Mnemonic( token[0], token[1] , &value , line_num, &inst_num);
		if( ret == 0 )
			ret = Is_Directive(  token[0], token[1] , &value , line_num, &inst_num );

		if( ret != Nothing ){

			if( Assemble_State.start == FALSE ){// START doesn't exist
				new_error = (error*)malloc(sizeof(error));
				new_error->line = line_num;
				strcpy( new_error->message, "Code must begin with START.");
				new_error->next = NULL;
				push_into_error_list( new_error );		
				Assemble_State.error = TRUE;
				Assemble_State.start = TRUE;
			}

			else if( arg_num > 2 ){// Too much Operand
				new_error = (error*)malloc(sizeof(error));
				new_error->line = line_num;
				strcpy( new_error->message, "This line has too many arguments.");
				new_error->next = NULL;
				push_into_error_list( new_error );		
				Assemble_State.error = TRUE;
			}
			else if( (ret == Directive) && ( inst_num == BYTE || inst_num == WORD || inst_num == RESB || inst_num == RESW )){
				new_error = (error*)malloc(sizeof(error));
				new_error->line = line_num;
				strcpy( new_error->message, " This line must have Label.");
				new_error->next = NULL;
				push_into_error_list( new_error );		
				Assemble_State.error = TRUE;
			}
			else{
				write_interm( line_num, LOCCTR, arg_num, comment, blank, token[0], ret , inst_num , token[1]);
				LOCCTR += value;
			}
		}
		else{
			// SYMBOL
			ret = push_symbol( token[0] , LOCCTR );
			if( ret == FALSE ){
				new_error = (error*)malloc(sizeof(error));
				new_error->line = line_num;
				strcpy( new_error->message, "fail to push into SYMTAB.");
				new_error->next = NULL;
				push_into_error_list( new_error );		
				Assemble_State.error = TRUE;
			}

			else{

				ret = Is_Mnemonic( token[1], token[2], &value, line_num, &inst_num);
				if( ret == 0)
					ret = Is_Directive(  token[1], token[2] , &value , line_num, &inst_num);

				if( ret == Nothing ) {
					new_error = (error*)malloc(sizeof(error));
					new_error->line = line_num;
					strcpy( new_error->message, "It is not mnemonic or directive.");
					new_error->next = NULL;
					push_into_error_list( new_error );		
					Assemble_State.error = TRUE;
				}

				else if( Assemble_State.start == FALSE ){
					new_error = (error*)malloc(sizeof(error));
					new_error->line = line_num;
					strcpy( new_error->message, "Code must begin with START.");
					new_error->next = NULL;
					push_into_error_list( new_error );		
					Assemble_State.error = TRUE;
					Assemble_State.start = TRUE;
				}

				else if( Assemble_State.end == TRUE ){
					new_error = (error*)malloc(sizeof(error));
					new_error->line = line_num;
					strcpy( new_error->message, "END must not have symbol.");
					new_error->next = NULL;
					push_into_error_list( new_error );		
					Assemble_State.error = TRUE;
				}

				else{
					write_interm( line_num, LOCCTR, arg_num, comment, token[0], token[1], ret , inst_num , token[2]);
					LOCCTR += value;

				}
			}
		}
		line_num += 5;
	}

	if( Assemble_State.end == FALSE ){// Error
		new_error = (error*)malloc(sizeof(error));
		new_error->line = line_num;
		strcpy( new_error->message, "not proper end of file.");
		new_error->next = NULL;
		push_into_error_list( new_error );		
		Assemble_State.error = TRUE;
	}
	
	fclose(fp);
}

void write_interm( int line_num, unsigned int LOCCTR, int arg_num, char comment[], char *symbol,  char *instruction, int inst_flag, int inst_num, char *operand){
	
	/* 	 Line_Num            LOCCTR            Symbol            (+)Instruction            (@,#)    Operand           Opcode
	   |  4space  | 4space | 4space | 4space | 6space | 4space |     7space     | 3space | 1space | 9space | 4space | 8space | 
	   [0]        [4]      [8]      [12]     [16]     [22]     [26]             [33]     [36]     [37]     [46]     [50]  
	*/

	int len;
	FILE *fp;
	char flag;

	if( Assemble_State.error == TRUE )
		return;

	fp = fopen( Assemble_State.OutputFileName , "a");

	if( arg_num == 0 ){
		fprintf(fp, "%s", comment);
		fclose(fp);
		return;
	}
	//Write Line Num
	Write_Hex(fp, (unsigned int)Line_num, 4);
	Write_Blank(fp, 4);

	// Write LOCCTR
	Write_Hex(fp, LOCCTR, 4);
	Write_Blank(fp, 4);

	//Write SYMBOL
	if( symbol != NULL ){
		fprintf(fp, "%s", symbol);
		len = 10 - strlen(symbol);
	}
	else
		len = 10;
	Write_Blank(fp, len);

	//Write instruction
	if( instruction != NULL ){
		fprintf(fp, "%s", instruction);
		len = 10 - strlen( instruction );
	}
	else
		len = 10;
	Write_Blank(fp, len);

	//Write Operand
	if( operand != NULL ){
		len = strlen(operand);
		flag = operand[0];
		if(flag == '#' || flag == '@'){
			fprintf(fp, "%s", operand);
			len = 14 - len;
		}
		else{
			fprintf(fp, " %s", operand);
			len = 13 - len;
		}
	}
	else
		len = 14;
	Write_Blank(fp, len );

	// Write Directive or Format
	Write_Hex( fp, (unsigned int)inst_flag, 1 );

	// Write Dir_num or Opcode
	Write_Hex( fp, (unsigned int)inst_num, 2 );

	fprintf(fp, "\n");


	fclose(fp);

}
int Is_Directive ( char *instruction , char *operand, unsigned int *inc, int line_num , int *dir_num){

	char *start = "START";
	char *byte = "BYTE";
	char *word = "WORD";
	char *resb = "RESB";
	char *resw = "RESW";
	char *end = "END";
	char *base = "BASE";
	char *nobase = "NOBASE";

	int state, i;
	error *new_error;
	unsigned int value;
	int len;
	int ret = Directive;
	
	if( strcmp( instruction, start ) == 0 ){// START
		
		Assemble_State.start = TRUE;

		if( operand == NULL){
			*inc = 0;
			value = 0;
		}
		else{
			state = Str_convert_into_Hex(operand, &value);
			if( state == FALSE){
				new_error = (error*)malloc(sizeof(error));
				new_error->line = line_num;
				strcpy( new_error->message, "operand has wrong character.");
				new_error->next = NULL;
				push_into_error_list( new_error );		
				Assemble_State.error = TRUE;
			}
			else
				*inc = value;
		}
		
		ret = push_symbol( instruction, value );
		if( ret == FALSE ){
			new_error = (error*)malloc(sizeof(error));
			new_error->line = line_num;
			strcpy( new_error->message, "fail to make SYMTAB.");
			new_error->next = NULL;
			push_into_error_list( new_error );		
			Assemble_State.error = TRUE;
		}
		*inst_num = START;
	}
	else if( strcmp( instruction, end) == 0 ){// END
		Assemble_State.end = TRUE;
		*inst_num = END;
	}

	else if( strcmp( instruction, base) == 0 ){// BASE
		*inc = 0;
		*inst_num = BASE;
	}
	else if( strcmp( instruction, nobase) == 0 ){// NOBASE
		*inc = 0;	
		*inst_num = NOBASE;
	}
		
	else if( strcmp( instruction, byte) == 0 ){// BYTE
		len = strlen( operand );
		if( operand[1] != '\'' || operand[len-1] != '\'' )
			Assemble_State.error = TRUE;
		if( operand[0] == 'C'){
			*inc = ( len - 3 );
		}	
		else if( operand[0] == 'X'){
			len -= 3;
			len = ( len + 1 ) / 2;
			*inc = len;
		}
		*inst_num = BYTE;
	}

	else if( strcmp( instruction, word ) == 0 ){// WORD
		*inc = 3;
		*inst_num = WORD;
	}

	else if( ( strcmp( instruction, resb ) && strcmp( instruction, resw )) != 0 ){// mnemonic error
		ret = Nothing;
	}
	else{
		len = strlen(operand);
		value = 0;
		state = TRUE;
		for( i = 0; i < len ; i++ ){
			if( operand[i] >= '0' && operand[i] <= '9' ){
				value *= 10;
				value += (unsigned int)( operand[i] - '0' );
			}
			else{
				state = FALSE;
				break;
			}
		}			
		if( state == FALSE ){//error
			new_error = (error*)malloc(sizeof(error));
			new_error->line = line_num;
			strcpy( new_error->message, "operand has wrong character.");
			new_error->next = NULL;
			push_into_error_list( new_error );
			Assemble_State.error = TRUE;
		}
		else if( strcmp( instruction , resb) == 0 ){// RESB
			*inc= value;
			*inst_num = RESB;
		}
		else if( strcmp( instruction , resw) == 0 ){// RESW
			*inc = ( 3 * value );
			*inst_num = RESW;
		}
	}
	return ret;
}

int Is_Mnemonic( char *mnemonic , char *operand, unsigned int *inc, int line_num, int *opcode){

	char revise_mne[10];
	char blank[2];

	error *new_error;

	int format;
	int ret = 0;
	int format_flag = FALSE;

	if( mnemonic[0] == '+' ){
		strcpy( revise_mne, mnemonic+1 );
		format_flag = TRUE;
	}
	else
		strcpy( revise_mne, mnemonic );
		
	format = find_opcode( revise_mne, opcode );

	if( opcode < 0 )// Cannot find in OPTAB
		return ret;

	if( format < 3 && format_flag == TRUE ){
		new_error = (error*)malloc(sizeof(error));
		new_error->line = line_num;
		strcpy( new_error->message, "This mnemonic doesn't have format 4.");
		new_error->next = NULL;
		push_into_error_list( new_error );		
		Assemble_State.error = TRUE;
		return ret;
	}
	
	switch( format ){
		case 1: {
					if( operand != NULL){
						new_error = (error*)malloc(sizeof(error));
						new_error->line = line_num;
						strcpy( new_error->message, "This mnemonic must not have operand.");
						new_error->next = NULL;
						push_into_error_list( new_error );		
						Assemble_State.error = TRUE;
					}
					*inc = 1;
					ret = Format1;
					break;
				}
		case 2: {
					if( operand == NULL ){
						new_error = (error*)malloc(sizeof(error));
						new_error->line = line_num;
						strcpy( new_error->message, "This mnemonic must have operand.");
						new_error->next = NULL;
						push_into_error_list( new_error );		
						Assemble_State.error = TRUE;
					}
					*inc = 2;
					ret = Format2;
					break;
				}
		case 3: {
					if( operand == NULL && strcmp( revise_mne, "RSUB" ) != 0 ){
						new_error = (error*)malloc(sizeof(error));
						new_error->line = line_num;
						strcpy( new_error->message, "This mnemonic must have operand.");
						new_error->next = NULL;
						push_into_error_list( new_error );		
						Assemble_State.error = TRUE;
					}
					if( format_flag == TRUE ){
						*inc = 4;
						ret = Format4;
					}
					else{
						*inc = 3;
						ret = Format3;
					}
					break;
				}
	}
	return ret;
}

int push_symbol( char *symbol, unsigned int LOCCTR ){

	int ret = TRUE;
	symbol_info *new_sym;
	symbol_info *pre, *cur;
	int Hash_value;
	int idx, len;

	// Is symbol consist of Upper Alphabet?
	len = strlen( symbol );
	for( idx = 0; idx < len; idx++ )
		if( symbol[idx] < 'A' || symbol[idx] > 'Z' )
			return FALSE;

	new_sym = (symbol_info *)malloc( sizeof(symbol_info) );
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

int Find_Symbol( char *symbol, unsigned int *target_addr ){

	symbol_info *cur;
	int Hash_value;
	int ret = FALSE;
	int state;

	Hash_value = Hash_func( symbol );
	
	cur = SYMTAB[Hash_value];
	while( cur != NULL ){
		state = strcmp( symbol, cur->name);
		if( state == -1 )
			cur = cur->next;
		else if( state == 0 ){
			ret = TRUE;
			*target_addr = cur->address;
			break;
		}
	}
	return ret;
}

void assem_pass2(char filename[], int code_len ){

	FILE *fp;
	FILE *fp2;
	


	char operand[10];
	unsigned int NI;
	int inst_flag;
	char inst_num[2];
	char object_code[200];
	char tmp[5];

	char line[200];
	char *pt;
	
	int len;
	unsigned int line_num=5;
	unsigned int next_line_num;
	unsigned int LOCCTR;
	unsigned int PC;

	error *new_error;

	fp = fopen(Assemble_State.OutputFileName, "r" );
	if( fp == NULL ){		
		new_error = (error*)malloc(sizeof(error));
		new_error->line = line_num;
		strcpy( new_error->message, "Intermediate file doesn't exist.");
		new_error->next = NULL;
		push_into_error_list( new_error );		
		Assemble_State.error = TRUE;
		return;
	}

	// outputfile initialize
	len = strlen( filename );
	len -= 4;
	filename[len] = '\0';
	
	strcpy( Assemble_State.OutputFileName, filename );
	strcpy( Assemble_State.OutputFileName2, filename );
	strcat( Assemble_State.OutputFileName, ".lst" );
	strcat( Assemble_State.OutputFileName2, ".obj");
	fp2 = fopen( Assemble_State.OutputFileName, "w");
	fclose( fp2 );
	fp2 = fopen( Assemble_State.OutputFileName2, "w");
	fclose( fp2 );

	// Make Header Code

	pt = fgets(line,200, fp);
	if( pt == NULL){
		new_error = (error*)malloc(sizeof(error));
		new_error->line = line_num;
		strcpy( new_error->message, "Intermediate file is empty.");
		new_error->next = NULL;
		push_into_error_list( new_error );		
		Assemble_State.error = TRUE;
		return;
	}

	// Get PC
	Get_Token( line, tmp, 8, 12 );
	Str_convert_into_Hex( tmp, &PC );

	while(TRUE){

		// Get LOCCTR
		LOCCTR = PC;

		// Get line number
		Get_Token( line, tmp, 0, 4);
		Str_convert_into_Hex( tmp, &next_line_num );

		// N I flag set
		NI = 1;
		if( line[36] == '@' )
			NI = NI<<1;
		else if( line[36] != '#' ){
			NI = NI<<1;
			NI += 1;
		}

		// Get operand
		Get_Token( line, operand, 37, 46 );

		// Get Instruction flag
		inst_flag = line[50]-'0';

		// Get Instruction num
		Get_Token( line, inst_num, 51,53 );

		while( TRUE ){
			pt = fgets( line, 200, fp );
			if( pt == NULL )
				break;
			if( line[0] != '.' )
				break;
			Write_lst( line, 0, FALSE);
		}
		
		// Get PC
		Get_Token( line, tmp, 8, 12 );
		Str_convert_into_Hex( tmp, &PC );

		Make_Object_Code( operand, NI, inst_flag, inst_num, 	

	



}

void Get_Token( char line[], char store[], int start_idx, int end_idx){
	int i;
	int idx = 0;

	for( i = start_idx ; i < end_idx ; i++, idx++ ){
		if( line[i] == ' ')
			break;
		store[idx] = line[i];
	}
	store[idx] = '\0';	
}

void Write_lst( char line[] , int LOC_chg, int ObjectCode){

	FILE *fp;
	fp = fopen( Assemble_State.OutputFileName, "a" );

	if( line[0] == '.' ){
		Write_Blank( fp, 4 );
		fprintf(fp, "%s", comment );
		fclose(fp);
		return;
	}

	if( ObjectCode == False){

		line[50] = '\0';

		if( LOC_chg == 0 ){
			line = line + 8;
			Write_Blank( fp, 4 );
			fprintf(fp, "%s", line);
		}
		else{
			line = line + 4;
			fprintf(fp, "%s", line);
		}
	}
	else{

		fprintf(fp,"\n");
	}

	fclose(fp);

}
int Make_Object_Code(){
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

int find_opcode( char mnemonic[], int *opcode){
	int adr;
	int format;
	opcode_info *cur;

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

	*opcode = (int)cur->opcode;
	
	for( format = 1; format <= 4; format++ )
		if( cur->format[format] == 1 )
			break;
	return format;
}
