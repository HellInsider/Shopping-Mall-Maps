# Code rules: 
  *Discussion -> Code -> Test -> Review -> Done*     
  1) In 3 days before deadline must be test and review.     
  2) No "magic" numbers and constants in code. Everything must be in enums, defines or special variables.   
  3) Use global variables only after team discussion.    

  # Code style:    
  1) Every figure "{" must be on new string. 
 ```C
      void foo(int a)
      {
	      ...
      }
 ```
  2) Single if-else must be with " { ... }".     
 ```C
      void foo(int a)
      {
	    if (n > 0)
        {
         ...
        }
        else
        {
        ...
        }
      }
 ```
  3) Naming.
  *	camelCase for: local variables, class members
  *	PascalCase for: functions
  *	snake_case for: data types (except types declared in the libraries used)
  *	UPPER_CASE for: global constants, enumerations elements, macro constants
 ```C
      const float PI_CONSTANT = 3.1415926;
      
      class my_class
      {
          int memberData;
	 	 void ProcessData();
      };
      
      void Foo(int a)
      {
		 int maxNumber;
          double ballForceThrow;
      }
      
      void OpenFile()
      {
      ...
      }
 ```
  4) Indentation
  *	Use only spaces, 4 spaces
  *	Any inner block must be indented, '{' must has same indentation as outer block
  *	Data access specifiers ("public", "protected", "private") has same indentation as class
  *	"case" markers has same indentation as "switch" block
  ```C
  class my_class
  {
  public:
      void ProcessData()
      {
          switch (memberData)
          {
          case 0:
              memberData++;
              break;
          case 1:
              memberData--;
              break;
          case 2:
              printf("Error");
              break;
          }
      }
  private:
      int memberData;
  };
  ```
  5) If function fits the screen, variables declares in the beginning, else in the place of use. 
  6) Insert spaces between operators and operands.
 ```C
      int x = (a + b) * c / d + foo();
 ```
 7) Each variable declaration on a new line.
  ```C
      int x = 3;
      int y = 7;
      double z = 4.25;
 ```
 8) When the line gets longer than 100 characters, divide it into two, making a newline after the operator, and continue writing.
 ```C
   int result = reallyLongFunctionOne() + reallyLongFunctionTwo() + 
        reallyLongFunctionThree() + reallyLongFunctionFour();
 ```
 9) Leave blank lines between functions and between expression groups.
  ```C
   void foo() 
   {
    ...
   }
                          // Empty line
   void bar() 
   {
    ...
   }
 ```
