#include "20171697.h"
#include "variable.h"

int Str_convert_into_Hex(char str[], unsigned int *num){
	int idx = 0;
	int ret = TRUE;
	unsigned int number = 0;
	
	if(str[0] == '\0')
		ret = FALSE;
	while(ret){
		if(str[idx] == '\0')
			break;
		else if( str[idx] >= '0' && str[idx] <= '9' ){
			number *= 16;
			number += (str[idx] - '0');
		}
		else if ( str[idx] >= 'a' && str[idx] <= 'f' ){
			number *= 16;
			number += (str[idx] - 'a' + 10);
		}
		else if ( str[idx] >= 'A' && str[idx] <= 'F' ){
			number *= 16;
			number += (str[idx] - 'A' + 10);
		}
		else{
			ret = FALSE;
			break;
		}
		idx++;
	}
	*num = number;
	return ret;
}
void Hex_convert_into_Str(unsigned int num, int len){
	char str[6];
	int idx;
	int remain;

	str[len] = '\0';
	for(idx = (len-1) ; idx >= 0 ; idx-- ){
		remain = num % 16;
		if( remain < 10 )
			str[idx] = remain + '0';
		else
			str[idx] = (remain - 10) + 'A';
		num /= 16;
	}
	printf("%s",str);
}

void Write_Hex(FILE *fp, unsigned int num, int len){
	char str[6];
	int idx;
	int remain;

	str[len] = '\0';
	for(idx = (len-1) ; idx >= 0 ; idx-- ){
		remain = num % 16;
		if( remain < 10 )
			str[idx] = remain + '0';
		else
			str[idx] = (remain - 10) + 'A';
		num /= 16;
	}
	fprintf(fp,"%s",str);
}

void Make_Hexa_String( unsigned int num, int len, char str[]){
	int i;
	int remain;

	for( i = len - 1 ; i >= 0 ; i-- ){
		remain = num % 16;
		num /= 16;
			if( remain < 10 )
			str[i] = remain + '0';
		else
			str[i] = ( remain - 10 ) + 'A';
		
	}
	str[len] = '\0';
}

void Write_Blank(FILE *fp, int len){
	int i;
	for( i = 0 ; i < len ; i++ )
		fprintf(fp, " ");
}


int Handling_Input(int mode, char input_str[], char str[], int len, int HEXA){
	/*mode=0: store until meeting blank
	  mode=1: erase blanks
	*/
	int idx = 0;
	int ret;
	int sig_exist = FALSE;
	int multi_zero = FALSE;
	char ch;
	while(1){
		ch = input_str[rd_pt];
		if(ch == '\n'){
			ret = ENTER;
			break;
		}
		else if(ch == ','){
			ret = COMMA;
			rd_pt++;
			break;
		}
		if((ch == ' ' || ch == '\t' || ch == '\r' || ch == '\v') != mode){
			if( mode == Store_input )
				ret = BLANK;
			else
				ret = CHAR;
			break;
		}
		if(mode == Store_input){
			if(idx >= len){
				ret = FALSE;
				break;
			}
			else if( HEXA && (idx == 1) && (str[0] == '0') ){
				if(ch == '0'){
					multi_zero = TRUE;
				}
				else if( (multi_zero == FALSE ) && ( ch == 'X' || ch == 'x') && (sig_exist == FALSE) ){
					sig_exist = TRUE;
					idx = 0;
				}
				else{
					str[0] = ch;
				}
			}
			else{
				str[idx] = ch;
				idx++;
			}
		}
		rd_pt++;
	}
	if(mode == Store_input)
		str[idx]='\0';
	return ret;
}


int Get_argument(unsigned int *arg1, unsigned int *arg2, unsigned int *arg3, int arg_len[]){
	int arg_num = 0;
	int Max_arg_num = 3;
	int i;
	int ret;
	int ret3, ret2, ret1;
	char argument[3][6];
	ret3 = ret2 = ret1 = TRUE;

	for( i = 0 ; i < Max_arg_num ; i++ ){//get one argument for one loop
		ret = Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE);
		if(ret == ENTER){
			if(i != 0)
				Success = FALSE;
			return arg_num;
		}
		else if(ret == COMMA){
			Success = FALSE;
			return arg_num;
		}
		ret = Handling_Input( Store_input, new_input->str, argument[i], arg_len[i], TRUE);
		arg_num++;
		if(ret == BLANK){
			ret = Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE);
		}
		if(ret == ENTER){
			switch(arg_num){
				case 3: ret3  = Str_convert_into_Hex( argument[2], arg3);
				case 2: ret2  = Str_convert_into_Hex( argument[1], arg2);
				case 1: ret1 = Str_convert_into_Hex( argument[0], arg1);
			}
			Success = ( ret3 && ret2 && ret1 );
			return arg_num;
		}
		else if(ret == CHAR || ret == FALSE){
			Success = FALSE;
			return arg_num;
		}
	}
	if(ret != ENTER)
		Success = FALSE;
	return arg_num;
}

int Get_String_Argument(char store[]){
	// return a type of character which position is end of reading
	int ret;

	ret = Handling_Input( Erase_space, new_input->str, NULL, 0 ,FALSE);
	
	if( ret != CHAR ){// argument doesn't exist
		Success = FALSE;
		return ret;
	}

	ret = Handling_Input( Store_input, new_input->str, store, MAX_COMMAND, FALSE);
	if ( ret == BLANK )
		ret = Handling_Input( Erase_space, new_input->str, NULL, 0 , FALSE);
	return ret;
}
