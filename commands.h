#ifndef COMMANDS_H
#define COMMANDS_H
/*Function: Counts the number of tokens in a String
  if it were to be separated by spaces.
  Return Value: Int number of pieces that the String
  can be separated into.
  Args:
  source: String that is used for counting         */
int count_tokens( char * source);

/*Function: Parses a string into parts by spaces.
  
  Return Value: A pointer that points to the first separated
  string.
  Args:
  source: String that is being split.                      */

char ** parse_by_space( char * source);

/*Function: Removes the white space at the front and back
  of a string.
  Return Value: A string that does not have the white space.
  Args:
  source: String that needs to be trimmed.
*/

char * trim( char * source );
#endif
