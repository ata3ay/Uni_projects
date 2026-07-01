//---------------------------------------------------------------------------------------------------------------------
/// a2.c
/// Intricate Square Placement
///
/// Author: 12046429 Atabay Dinmukhamed
//---------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
// #include "framework.h"

#define ROWS 5
#define COLUMNS 5
#define SIZE 5 
#define REMAINING_PIECES 4
#define PLAYER_1 1
#define PLAYER_2 2
#define CONTINUE_GAME 3
#define START_TARGETING 5

//---------------------------------------------------------------------------------------------------------------------
/// @brief enum values for returning
/// 
///
/// 
//---------------------------------------------------------------------------------------------------------------------
enum INVALID_INPUT
{
  QUIT = 0,
  ERROR_UNKNOWN_COMMAND = -1,
  ERROR_REMAINING_PIECES = -2,
  ERROR_COORDINATES_NOT_ON_BOARD = -3,
  ERROR_NO_SHIFTABLE_PIECE = -4,
  ERROR_FIELD_OCCUPIED = -5,
  ERROR_NOT_ADJASENT = -6,
  ERROR_SPECIFY_PIECE = -7
};

//---------------------------------------------------------------------------------------------------------------------
/// @brief enum values for winning
/// 
///
/// 
//---------------------------------------------------------------------------------------------------------------------
enum WINNING_VALUES
{
  PLAYER_ONE_WON = 1,
  PLAYER_TWO_WON = 2
};

//---------------------------------------------------------------------------------------------------------------------
/// @brief struct for Player that plays the game
/// 
///
/// 
//---------------------------------------------------------------------------------------------------------------------
typedef struct _Player_
{
  int player_id_;
  int move_number_;
  int remaining_pieces_;
  int last_row_;
  int last_column_;
  int winner_;
} Player;

//---------------------------------------------------------------------------------------------------------------------
/// @brief prints the welcome message at the beginning of the game
/// 
///
/// 
//---------------------------------------------------------------------------------------------------------------------
void welcomeMessage(void)
{
  printf("===| Welcome to       |===\n");
  printf("===| [I]ntricate      |===\n");
  printf("===| [S]quare         |===\n");
  printf("===| [P]lacement      |===\n\n");
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief prints the remaining pieces for every player after the turn
/// 
///
/// 
//---------------------------------------------------------------------------------------------------------------------
void remainingPiecesMessage(Player player_one, Player player_two)
{
  printf("---| Remaining Pieces |---\n");
  printf("\nPlayer 1: %d\n", player_one.remaining_pieces_);
  printf("Player 2: %d\n\n", player_two.remaining_pieces_);
}

//---------------------------------------------------------------------------------------------------------------------
/// 
/// @brief prints the move message of the current player
///
/// 
//---------------------------------------------------------------------------------------------------------------------
void moveMessage(Player player, int count)
{
  printf("\n===| Move %d: Player %d |===\n\n", count, player.player_id_);
}

//---------------------------------------------------------------------------------------------------------------------
/// 
/// @brief prints the message where it asks for the input
/// @param struct player
/// @param count
//---------------------------------------------------------------------------------------------------------------------
void askForInputMessage (Player player)
{
  printf("P%d > ", player.player_id_);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief prints the corresponding error message according to the enum value
/// @param error_code
///
/// 
//---------------------------------------------------------------------------------------------------------------------
void printError(int error_code)
{
  if(error_code == ERROR_UNKNOWN_COMMAND)
  {
    printf("\n[ERROR] Unknown command.\n\n");
  }
  if(error_code == ERROR_COORDINATES_NOT_ON_BOARD)
  {
    printf("\n[ERROR] Coordinates not within board.\n\n");
  }
  if(error_code == ERROR_FIELD_OCCUPIED)
  {
    printf("\n[ERROR] Field at coordinates occupied.\n\n");
  }
  if(error_code == ERROR_NO_SHIFTABLE_PIECE)
  {
    printf("\n[ERROR] No shiftable piece at coordinates.\n\n");
  }
  if(error_code == ERROR_NOT_ADJASENT)
  {
    printf("\n[ERROR] Field at coordinates not adjacent to chosen piece.\n\n");
  }
  if(error_code == ERROR_SPECIFY_PIECE)
  {
    printf("\n[ERROR] You must specify a piece to shift and a target field.\n\n");
  }
  if(error_code == ERROR_REMAINING_PIECES)
  {
    printf("\n[ERROR] You must first place all your remaining pieces.\n\n");
  }

}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Removes leading and trailing whitespace and compresses internal. THe function was made with the help of ChatGPT
/// whitespace sequences to a single space. "    a1   b1   " -> "a1 b1"
/// @param char *string
/// 
/// 
//---------------------------------------------------------------------------------------------------------------------
// Removes leading and trailing whitespace and compresses internal
// whitespace sequences to a single space.
// Example:
// "    a1   b1   " -> "a1 b1"
// "   a1    "     -> "a1"
// "B1          "  -> "B1"
void normalizeInput(char *s)
{
  int read = 0;
  int write = 0;
  int space_pending = 0;

  // Skip leading whitespace
  while (s[read] != '\0' && isspace((unsigned char)s[read]))
  {
    read++;
  }

  while (s[read] != '\0')
  {
    if (isspace((unsigned char)s[read]))
    {
        space_pending = 1;
    }
    else
    {
        // only add a single space between tokens
      if(space_pending && write > 0)
      {
        s[write++] = ' ';
      }
      space_pending = 0;
      s[write++] = s[read];
    }
    read++;
  }
  s[write] = '\0';
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief gets input from the user dynamically
/// 
///
/// 
//---------------------------------------------------------------------------------------------------------------------
char* getInput(void)
{
  int buffer = 50; 
  int index = 0;
  char* get_input = malloc(buffer * sizeof(char));
  if(get_input == NULL)
  {
    return NULL;
  }
  char* symbol_now = get_input; // temp variable to scroll through the string // hello h e l l o \0
  while (1)
  {
    int ch = getchar();// takes the symbols from the input buffer
    if(ch == EOF)
    {
      if(index == 0)
      {
        free(get_input);
        return NULL;
      }
      else
      {
        break;
      }
    }
    if(ch == '\n') // it ends when the new line appears
    {
      break;
    }
    if(index + 1 >= buffer)
    {
      buffer = buffer * 2;
      char* temp = realloc(get_input, buffer * sizeof(char));/// we need temp for safety and to avoid memory leak
      if(temp == NULL) /// if reallocation fails we lose our pointer
      {
        printf("Reallocation failed\n");
        free(get_input);
        return NULL;
      }
      get_input = temp; ///overwriting after making sure that it is safe to allocate memory
      symbol_now = get_input + index; 
    }
    *symbol_now++ = ch;// ptr goes to next symbol, postincremeant, first operation
    index++;
  } // symbol_now points to the last character at the moment
  *symbol_now = '\0';///i have to write null terminator, to finish the string
  return get_input;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief makes the input with lowercase
/// @param char* input_argv 
///
/// 
//---------------------------------------------------------------------------------------------------------------------
void toLower(char* input_argv)
{
  int length = strlen(input_argv);

  if(*input_argv == '\0')
  {
    return;
  }

  for(int i = 0; i < length; i++)
  {
    input_argv[i] = tolower(input_argv[i]);
  }
}


//---------------------------------------------------------------------------------------------------------------------
/// @brief parses the arguments 
/// @param arg count number in the command line
/// @param char* argv 
/// @param bool color_status for the board
/// @return return -1 or 0
//---------------------------------------------------------------------------------------------------------------------
int parseArguments(int argc, char* argv[], bool *color_status) // ptr = original data
{
  *color_status = false; //default settings

  if(argc > 2)
  {
    printf("\n[ERROR] Usage: ./a2 [optional: color_enabled]\n\n");
    return -1;
  }

  if(argc == 2)
  {
    toLower(argv[1]);
    if(strcmp(argv[1], "enabled") == 0)
    {
      *color_status = true;
    }
    else if(strcmp(argv[1], "disabled") == 0)
    {
      *color_status = false;
    }
    else
    {
      printf("\n[ERROR] (Optional) color output argument can either be \"disabled\" or \"enabled\".\n\n");
      return -1;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief winning pattern for the final game state
/// @param struct player
/// @param board
/// @param color_status of the board
/// 
//---------------------------------------------------------------------------------------------------------------------
void winningPattern (Player player, int board[SIZE][SIZE], bool color_status)
{
  printf("\n===| Final Game State |===\n\n");
  printBoard(SIZE, board, color_status, PLAYER_1, PLAYER_2);
  printf("===| Winner: Player %d |===\n", player.player_id_);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief checks the input coordinates while shifting first 4 pieces if they are given right
/// @param input
/// @return returns the error enum if its wrong, and 1 as success
/// 
//---------------------------------------------------------------------------------------------------------------------
int checkInputCoordinates(char* input) //shifting
{
  int length = strlen(input);
  if(length != 5 && length != 2 && length != 4) // Invalid 3 commands
  {
    return ERROR_UNKNOWN_COMMAND; 
  }

  for(int i = 0; i < length; i++)
  {
    input[i] = tolower(input[i]);
  }

  if(length == 4) // expect quit input
  {
    if(strcmp("quit", input) == 0)
    {
      return QUIT;
    } 
    return ERROR_UNKNOWN_COMMAND; //unknown command
  }
  if(length == 5)
  {
    char in_1 = input[0];
    char in_2 = input[1];
    
    char space = input[2];
    char in_3 = input[3];
    char in_4 = input[4];
    if(!isdigit(in_2) || !isdigit(in_4))
    {
      return ERROR_UNKNOWN_COMMAND;
    }
    if(space != ' ')
    {
      return ERROR_UNKNOWN_COMMAND;
    }

    if(in_1 < 'a' || in_1 > 'e' || in_2 < '1' || in_2 > '5'|| in_3 < 'a' || in_3 > 'e' 
      || in_4 < '1' || in_4 > '5')
    {
      return ERROR_REMAINING_PIECES;
    }
    return ERROR_REMAINING_PIECES;
  }
  if(length == 2)
  {
    char column = input[0];
    char row = input[1];
    if((column >= '1' && column <= '5')|| (row >= 'a' && row <= 'z'))
    {
      return ERROR_UNKNOWN_COMMAND;
    }
    if(column < 'a' || column > 'e' || row < '1' || row > '5')
    {
      return ERROR_COORDINATES_NOT_ON_BOARD;
    }
  }
  return 1;
}


//---------------------------------------------------------------------------------------------------------------------
/// @brief checks the input coordinates while targeting and shifting
/// @param input
/// @return returns error enum, or 1 as success
/// 
//---------------------------------------------------------------------------------------------------------------------
int fromToValidateCoordinates(char* input)
{
  int length = strlen(input);
  if(length != 5 && length != 2 && length != 4) // Invalid 3 commands
  {
    return ERROR_UNKNOWN_COMMAND; 
  }
  for(int i = 0; i < length; i++)
  {
    input[i] = tolower(input[i]);
  }

  if(strcmp("quit", input) == 0)
  {
    return QUIT;
  }

  if(length == 2)
  {
    char column = input[0];
    char row = input[1];
    if((column >= '1' && column <= '5')|| (row >= 'a' && row <= 'z'))
    {
      return ERROR_UNKNOWN_COMMAND;
    } 
    else 
    {
      return ERROR_SPECIFY_PIECE;
    }
    
  }
  
  if(length != 5)
  {
    return ERROR_UNKNOWN_COMMAND;
  }

  char in_1 = input[0];
  char in_2 = input[1];
  char space = input[2];
  char in_3 = input[3];
  char in_4 = input[4];
  if(!isdigit(in_2) || !isdigit(in_4))
  {
    return ERROR_UNKNOWN_COMMAND;
  }
  if(space != ' ')
  {
    return ERROR_UNKNOWN_COMMAND;
  }

  if(in_1 < 'a' || in_1 > 'e' || in_2 < '1' || in_2 > '5'|| in_3 < 'a' || in_3 > 'e' 
    || in_4 < '1' || in_4 > '5')
  {
    return ERROR_COORDINATES_NOT_ON_BOARD;
  }
  return 1;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief validates for the erros while shifting and targeting. In this function are some specific errors, f.e
/// erros if the piece does not belong to the player, or field is occupied
/// @param board
/// @param currents players id
/// @param from_col
/// @param from_row
/// @param to_col
/// @param to_row
//---------------------------------------------------------------------------------------------------------------------
int checkErrors(int board[SIZE][SIZE], int current_player, int* from_col, int* from_row, int* to_col, int* to_row)
{
  
  if(board[*from_row][*from_col] != current_player)
  {
    if(board[*from_row][*from_col] == 0)
    {
      // if there is no piece
      printError(ERROR_NO_SHIFTABLE_PIECE);
      return ERROR_NO_SHIFTABLE_PIECE;
    } 
    else
    {
      // opponents piece
      printError(ERROR_NO_SHIFTABLE_PIECE);
      return ERROR_NO_SHIFTABLE_PIECE;
    }
  }
  
  if(board[*to_row][*to_col] != 0)
  {
    printError(ERROR_FIELD_OCCUPIED);
    return ERROR_FIELD_OCCUPIED;
    
  }
  
  int row_cell = *to_row - *from_row; // the difference
  int column_cell = *to_col - *from_col; // the difference

  if((row_cell > 1 || row_cell < -1) || (column_cell > 1 || column_cell < -1))
  {
    printError(ERROR_NOT_ADJASENT);
    return ERROR_NOT_ADJASENT;
  }
  else
  {
    board[*to_row][*to_col] = board[*from_row][*from_col];
    board[*from_row][*from_col] = 0;
    return 1; //success
  }
  return ERROR_UNKNOWN_COMMAND;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief checks for the quadrat pattern for winning
/// @param struct player
/// @param board
/// @return returns 1 as success and 0 for failing
//---------------------------------------------------------------------------------------------------------------------
int checkQuadratPattern(Player player, int board[SIZE][SIZE])
{
  int last_row = player.last_row_;
  int last_column = player.last_column_;
  int id = player.player_id_;

  int directions[4][2] = {
    {last_row, last_column}, //top left
    {last_row - 1, last_column}, // bottom-left
    {last_row, last_column - 1}, // top-rigth
    {last_row - 1, last_column - 1} // bottom right
  };

  for(int i = 0; i < 4; i++)
  {
    int top_left_row = directions[i][0];
    int top_left_corner = directions[i][1];

    if(top_left_row < 0 || top_left_corner < 0)
    {
      continue;
    }
    if(top_left_row + 1 >= SIZE || top_left_corner + 1 >= SIZE)
    {
      continue;
    }

    if(board[top_left_row][top_left_corner] == id && board[top_left_row][top_left_corner + 1] == id 
      && board[top_left_row + 1][top_left_corner] == id && board[top_left_row + 1][top_left_corner + 1]== id)
    {
      return 1;
    }
  }
  return 0;
}

 int checkWinningPatternTwo(Player player, int board[SIZE][SIZE], int directions[4][2], int row, int column)
 {
  const int WINNER_SCORE = 4;
  const int PIECE_DIRECTIONS = 4;
  for(int direction = 0; direction < PIECE_DIRECTIONS; direction++)
  {
    int direct_row = directions[direction][0]; // x achse
    int direct_column = directions[direction][1]; // y achse
    int count = 1;
    for(int j = 1; j < WINNER_SCORE; j++)
    {
      int x = row + direct_row * j;
      int y = column + direct_column * j;

      if(x < 0 || x >= SIZE || y < 0 || y >= SIZE)
      {
        break;
      }

      if(board[x][y] == player.player_id_)
      {
        count++;
      }
      else
      {
        break;
      }
      if(count == WINNER_SCORE)
      {
        return 1;
      }
    }
  }
  return 0;
 }

//---------------------------------------------------------------------------------------------------------------------
/// @brief checks for the winning patterns horizontal, diagonal, vertical
/// @param struct player
/// @param board
/// @return return 1 for winning and 0 there is no winner
//---------------------------------------------------------------------------------------------------------------------
int checkWinningPattern(Player player, int board[SIZE][SIZE])
{
  int directions[4][2] = {
    {0, 1},
    {1, 0},
    {1, 1},
    {-1, 1}
  };

  int quadrat_check = checkQuadratPattern(player, board);
  if(quadrat_check == 1)
  {
    return 1;
  }

  for(int row = 0; row < SIZE; row++)
  {
    for(int column = 0; column < SIZE; column++)
    {
      if(board[row][column] != player.player_id_)
      {
        continue; 
      }
      int has_won = checkWinningPatternTwo(player, board, directions, row, column);
      if(has_won == 1)
      {
        return 1;
      }
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief places one piece of the player
/// @param struct player
/// @param column current to store the coordinates
/// @param row current to store the coordinates
/// @return returns 1 for success or enum value quit
//---------------------------------------------------------------------------------------------------------------------
int placingOnePiece(Player player, int* column, int* row)
{
  int input_check;
  char* input;
  while(1)
  {
    askForInputMessage(player);
    input = getInput();
    if(input == NULL)
    {
      return QUIT;
    }
    normalizeInput(input);
    input_check = checkInputCoordinates(input);
    if(input_check == QUIT)
    {
      free(input);
      return QUIT;
    }
    if(input_check != 1)
    {
      printError(input_check);
      free(input);
      continue;
    }
    if(input_check == 1)
    {
      *column = toupper(input[0]) - 'A'; // to store the coordinates
      *row = input[1] - '1';
      free(input);
      return 1;
    }
  }
}


//---------------------------------------------------------------------------------------------------------------------
/// @brief places 4 pieces of the players
/// @param struct player
/// @param struct player
/// @param board
/// @param color_status
/// @param row
/// @param column
/// @return returns enum value, or contniues the game
//---------------------------------------------------------------------------------------------------------------------
int placingFourPieces(Player *player_one, Player *player_two,int board[SIZE][SIZE], bool color_status,
                      int* row, int* column)
{
  int place;
  player_one->remaining_pieces_ = REMAINING_PIECES;
  player_two->remaining_pieces_ = REMAINING_PIECES;
  for(int i = 1; i <= REMAINING_PIECES; i++)
  {
    moveMessage(*player_one, i); // Player 1 turn
    printBoard(SIZE, board, color_status, player_one->player_id_, player_two->player_id_);
    remainingPiecesMessage(*player_one, *player_two);
    while(1)
    {
      place = placingOnePiece(*player_one, column, row);
      if(place == 0)
      {
        return QUIT; // for quit
      }
      if(board[*row][*column] != 0)
      {
        printError(ERROR_FIELD_OCCUPIED);
        continue;
      }
      board[*row][*column] = PLAYER_1;
      player_one->remaining_pieces_--;
      player_one->last_row_ = *row;
      player_one->last_column_ = *column;
      break;
    }
    if(i >= REMAINING_PIECES)
    {
      player_one->winner_ = checkWinningPattern(*player_one, board);
      if(player_one->winner_ == 1)
      {
        winningPattern(*player_one, board, color_status);
        return PLAYER_ONE_WON;
      }
    }
    moveMessage(*player_two, i); // Player 2 turn
    printBoard(SIZE, board, color_status, PLAYER_1, PLAYER_2);
    remainingPiecesMessage(*player_one, *player_two);
    while(1)
    {
      place = placingOnePiece(*player_two, column, row);
      if(place == 0)
      {
        return 0; //for quit
      }
      if(board[*row][*column] != 0)
      {
        printError(ERROR_FIELD_OCCUPIED);
        continue;
      }
      board[*row][*column] = PLAYER_2;
      player_two->remaining_pieces_--;
      player_two->last_row_ = *row;
      player_two->last_column_ = *column;
      break;
    }
    if(i >= REMAINING_PIECES)
    {
      player_two->winner_ = checkWinningPattern(*player_two, board);
      if(player_two->winner_ == 1)
      {
        winningPattern(*player_two, board, color_status);
        return PLAYER_TWO_WON;
      }
    }
  }
  return CONTINUE_GAME;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief targets one piece
/// @param struct player
/// @param from_col
/// @param from_row
/// @param to_col
/// @param to_row
/// @return returns enum value, or 1 as a success
//---------------------------------------------------------------------------------------------------------------------
int targetingOnePiece(Player player, int* from_col, int* from_row, int* to_col, int* to_row)
{
  int input_check;
  char* input;

  while(1)
  {
    askForInputMessage(player);
    input = getInput();
    if(input == NULL)
    {
      return QUIT;
    }
    normalizeInput(input);
    input_check = fromToValidateCoordinates(input);
    if(input_check == QUIT)
    {
      free(input);
      return QUIT;
    }
    if(input_check != 1)
    {
      printError(input_check);
      free(input);
      continue;
    }
    *from_col = input[0] - 'a';
    *from_row = input[1] - '1';
    *to_col = input[3] - 'a';
    *to_row = input[4] - '1';
    free(input);
    return 1;
  }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief targeting and shifting of the pieces
/// @param struct player_one
/// @param struct player_two
/// @param board
/// @param color_status
/// @return returns the enum value, or 0
//---------------------------------------------------------------------------------------------------------------------
int targetingPieces(Player *player_one, Player *player_two,int board[SIZE][SIZE], bool color_status)
{
  int count = START_TARGETING; // because it starts at move 5
  int from_col;
  int from_row;
  int to_col;
  int to_row;
  while(player_one->winner_ != 1 && player_two->winner_ != 1)
  {
    moveMessage(*player_one, count);
    printBoard(SIZE, board, color_status, PLAYER_1, PLAYER_2);
    while(1)
    {
      int place = targetingOnePiece(*player_one, &from_col, &from_row, &to_col, &to_row);
      if(place == 0)
      {
        return QUIT;
      }
      int check_error = checkErrors(board, player_one->player_id_, &from_col, &from_row, &to_col, &to_row);
      if(check_error == 1)
      {
        player_one->last_row_ = to_row;
        player_one->last_column_ = to_col;
        break;
      }
    }
    player_one->winner_ = checkWinningPattern(*player_one, board);
    if(player_one->winner_ == 1)
    {
      winningPattern(*player_one, board, color_status);
      return PLAYER_ONE_WON;
    }
    moveMessage(*player_two, count);
    printBoard(SIZE, board, color_status, PLAYER_1, PLAYER_2);
    while(1)
    {
      int place = targetingOnePiece(*player_two, &from_col, &from_row, &to_col, &to_row);
      if(place == 0)
      {
        return QUIT;
      }
      int check_error = checkErrors(board, player_two->player_id_, &from_col, &from_row, &to_col, &to_row);
      if(check_error == 1)
      {
        player_two->last_row_ = to_row;
        player_two->last_column_ = to_col;
        break;
      }
    }
    player_two->winner_ = checkWinningPattern(*player_two, board);
    if(player_two->winner_ == 1)
    {
      winningPattern(*player_two, board, color_status);
      return PLAYER_TWO_WON;
    }
    count++;
  }
  return 0;
} 

//---------------------------------------------------------------------------------------------------------------------
/// @brief gaming loop, where all the game is played
/// @param struct player_one
/// @param struct player_two
/// @param color_status
/// @return return 0 - quit, 1 winner, 2 winner, 0 no winner and error
//---------------------------------------------------------------------------------------------------------------------
int gamingLoop(Player player_one, Player player_two, bool color_status)
{
  int board[ROWS][COLUMNS] = {0};
  int row;
  int column; 
  player_one.player_id_ = PLAYER_1;
  player_two.player_id_ = PLAYER_2;
  player_one.winner_ = 0;
  player_one.winner_ = 0;
  player_one.remaining_pieces_ = 0;
  player_two.remaining_pieces_ = 0;
  player_one.last_row_ = 0;
  player_two.last_row_ = 0;
  player_one.last_column_ = 0;
  player_two.last_column_ = 0;
  int four_pieces_placed = placingFourPieces(&player_one, &player_two, board, color_status, &row, &column);
  if(four_pieces_placed == QUIT)
  {
    return 0;
  }
  if(four_pieces_placed == PLAYER_ONE_WON)
  {
    return 1;
  }
  if(four_pieces_placed == PLAYER_TWO_WON)
  {
    return 2;
  }
  if(four_pieces_placed == 3)
  {
    int has_won = targetingPieces(&player_one, &player_two, board, color_status);
    if(has_won == QUIT)
    {
      return 0;
    } else if (has_won == PLAYER_ONE_WON)
    {
      return 1;
    } else if (has_won == PLAYER_TWO_WON)
    {
      return 2;
    }
    else
    {
      printError(has_won);
      return 0;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief main of this game
/// @param argument counts the arguments in the command line
/// @param argument vector
/// @return return -1 if the command line was not right handled, and returns the winner
//---------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  Player player_one = {0};
  Player player_two = {0};
  bool color_status; 

  int color = parseArguments(argc, argv, &color_status);
  if(color != 0)
  {
    return -1;
  }
 
  welcomeMessage();
  int winner_is = gamingLoop(player_one, player_two, color_status);
  return winner_is;
}



