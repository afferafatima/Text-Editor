#include <iostream>
#include <list>
#include <stack>
#include <deque>
#include <fstream>
#include <string>
#include <windows.h>
#include <conio.h>
using namespace std;
bool fileExist(string fileName);
void readFiles(string fileName);
void writeFiles(string newFile, string fileName);
void gotoRowColomn(int rpos, int cpos)
{
    COORD scrn;
    HANDLE hOuput = GetStdHandle(STD_OUTPUT_HANDLE);
    scrn.X = cpos;
    scrn.Y = rpos;
    SetConsoleCursorPosition(hOuput, scrn);
}
list<string> files;
string fileName = "test.txt";
struct state
{
    list<list<char>> text;
    list<list<char>>::iterator rowItr;
    list<char>::iterator colItr;
    int row, col;
};
class textEditor
{
public:
    int currentRow, currentCol; // current indices
    list<list<char>> text;
    list<list<char>>::iterator rowItr;
    list<char>::iterator colItr;
    deque<state> undo;
    stack<state> redo;
    textEditor()
    {
        text.push_back(list<char>());
        rowItr = text.begin();
        (*rowItr).push_back('\n');
        colItr = (*rowItr).begin();
        currentRow = 0;
        currentCol = 0;
    }
    state getState()
    {
        state *s = new state;
        s->text.push_back(list<char>());
        auto r_itr = s->text.begin(); // iterator of state s

        // Iterate through each row in the currentFile's text(deep copy)
        for (auto row = text.begin(); row != text.end(); r_itr++, ++row)
        {
            // Create a new row in the state's text
            s->text.push_back(list<char>());

            // Iterate through each character in the row
            for (auto col = (*row).begin(); col != (*row).end(); ++col)
            {
                // Add a new character to the state's row
                char ch = *col;
                (*r_itr).push_back(*col);
            }
        }

        // Find the row in the state's text that corresponds to the currentRow
        s->rowItr = s->text.begin();
        advance(s->rowItr, currentRow); // loop

        // Find the column in the state's row that corresponds to the currentCol
        s->colItr = (*s->rowItr).begin();
        advance(s->colItr, currentCol); // loop

        // Set the row and column indices in the state
        s->row = currentRow;
        s->col = currentCol;

        return *s;
    }
    void loadState(state s)
    {
        text = s.text;
        rowItr = text.begin();
        currentRow = s.row;
        currentCol = s.col;
        for (auto row = 0; row < s.row; row++)
        {
            rowItr++;
        }
        colItr = (*rowItr).begin();
        for (auto col = 0; col < s.col; col++)
        {
            colItr++;
        }
    }
    void undoOperation()
    {
        if (undo.empty())
        {
            // cout << "No undo operation available" << endl;
            return;
        }
        if (undo.size() > 5)
        {
            undo.pop_front();
        }
        redo.push(getState());
        state s = undo.back();
        undo.pop_back();
        loadState(s);
    }
    void redoOperation()
    {
        if (redo.empty())
        {
            // cout << "No redo operation available" << endl;
            return;
        }
        undo.push_back(getState());
        state s = redo.top();
        redo.pop();
        loadState(s);
    }
    // delete key
    void deleteOperation()
    {
        if (currentCol == (*rowItr).size() - 1)
        {
            if (currentRow==text.size()-1)// rowItr == text.end() not working
            {
                // cout << "No character to delete" << endl;
                return;
            }
            else
            {
                (*rowItr).pop_back();//removes last endline character
                auto nextRowItr = next(rowItr);
                (*rowItr).splice((*rowItr).end(), *nextRowItr);
                text.erase(nextRowItr);
                undo.push_back(getState());
            }
        }
        else
        {
            (*rowItr).erase(colItr);
            colItr=(*rowItr).begin();
            advance(colItr,currentCol);
        }
        gotoRowColomn(currentRow, currentCol);
    }
    // left arrow key
    void leftOperation()
    {
        if (colItr == (*rowItr).begin())
        {
            if (rowItr == text.begin())
            {
                // cout << "No character to move left" << endl;
                return;
            }
            else
            {
                rowItr--;
                colItr = (*rowItr).begin();
                currentRow--;
                advance(colItr, (*rowItr).size() - 1);
                currentCol = (*rowItr).size() - 1;
            }
        }
        else
        {
            colItr--;
            currentCol--;
        }
        gotoRowColomn(currentRow, currentCol);
    }
    // right arrow key
    void rightOperation()
    {

        colItr++;
        if (colItr == (*rowItr).end())
        {
            if (currentRow == text.size() - 1) // rowItr==text.end() not working
            {
                // cout << "No character to move right" << endl;
                colItr--;
                return;
            }
            else
            {
                rowItr++;
                colItr = (*rowItr).begin();
                currentRow++;
                currentCol = 0;
            }
        }
        else
        {

            // colItr++;done it at start
            currentCol++;
        }
        gotoRowColomn(currentRow, currentCol);
    }
    // up arrow key
    void upOperation()
    {
        if (rowItr == text.begin())
        {
            // cout<<"No character to move up";
            return;
        }
        rowItr--;
        if (currentCol > (*rowItr).size() - 1)
        {
            currentCol = (*rowItr).size() - 1;
            advance(colItr, currentCol);
        }
        else if (currentCol < (*rowItr).size())
        {
            colItr = (*rowItr).begin();
            advance(colItr, currentCol);
        }
        currentRow--;
        gotoRowColomn(currentRow, currentCol);
    }
    // down key
    void downOperation()
    {
        if (currentRow >= text.size() - 1) // already at the last row
        {
            // cout<<"No character to move down";
            return;
        }
        rowItr++;
        if (currentCol > (*rowItr).size() - 1)
        {
            currentCol = (*rowItr).size() - 1;
            colItr = (*rowItr).begin();
            advance(colItr, currentCol);
        }
        else if (currentCol < (*rowItr).size())
        {
            colItr = (*rowItr).begin();
            advance(colItr, currentCol);
        }
        currentRow++;
        gotoRowColomn(currentRow, currentCol);
    }
    // enter key
    void newLineOperation()
    {
        // Insert a newline character at the current position
        (*rowItr).insert(colItr, '\n');

        // Create a new line with the content after the newline character
        auto temp = rowItr;
        auto newRowItr = std::next(rowItr);
        list<char> newRow(colItr, (*rowItr).end());
        (*rowItr).erase(colItr, (*rowItr).end());

        // Insert the new line after the current line
        if (!newRow.empty())
        {
            text.insert(newRowItr, newRow);
        }

        // Move the iterators and update counters
        currentRow++;
        rowItr = text.begin();
        advance(rowItr, currentRow);
        colItr = (*rowItr).begin();
        currentCol = 0;

        undo.push_back(getState());
    }
    
    void backSpaceOperation()
    {
        undo.push_back(getState());

        if (colItr == (*rowItr).begin())
        {
            if (rowItr != text.begin())
            {
                auto prevRowItr = prev(rowItr);
                (*prevRowItr).pop_back(); // remove the end line character
                // Move the cursor to the end of the previous line
                colItr = (*prevRowItr).begin();
                currentCol = (*prevRowItr).size();
                
                // Merge the current line into the previous line
                (*prevRowItr).splice((*prevRowItr).end(), *rowItr);

                // Erase the current line
                text.erase(rowItr);

                // Update iterators and counters
                advance(colItr, currentCol);
                rowItr = prevRowItr;
                currentRow--;
            }
        }
        else
        {
            
            colItr--;
            colItr = (*rowItr).erase(colItr);
            currentCol--;
        }
    }
    //main input function
    void input()
    {
        char ch;
        while (true)
        {
            printText();
            ch = _getch();
            // if (ch == 26) // ctrl+z
            // {
            //     undoOperation();
            // }
            // else if (ch == 25) // ctrl+y
            // {
            //     redoOperation();
            // }
            if (ch == 19) // ctrl+s
            {
                save();
            }
            // else if (ch == 13) // enter key
            // {
            //     newLineOperation();
            // }
            else if (ch == 8) // backspace
            {
                backSpaceOperation();
            }
            if (ch == 27) // escape
            {
                break;
            }
            else if (ch == -32)
            {
                ch = _getch();
                if (ch == 75) // left arrow
                {
                    leftOperation();
                }
                else if (ch == 77) // right arrow
                {
                    rightOperation();
                }
                else if (ch == 72) // up arrow
                {
                    upOperation();
                }
                else if (ch == 80) // down arrow
                {
                    downOperation();
                }
                else if (ch == 83) // delete key
                {
                    deleteOperation();
                }
            }
            else
            {
                insertOperation(ch);
            }
        }
    }
    //insert in notepad
    void insertOperation(char ch)
    {
        if (isValidInput(ch))
        {
            size_t offset = distance((*rowItr).begin(), colItr);

            (*rowItr).insert(colItr, ch);
            colItr = (*rowItr).begin();
            colItr = next(colItr, offset); // points to the same character that is inserted
            colItr++;
            currentCol++;
        }
    }
    void save(string filename = "test.txt")
    {
        ofstream file;
        file.open(fileName);
        for (auto itr = text.begin(); itr != text.end(); itr++)
        {
            for (auto itr2 = (*itr).begin(); itr2 != (*itr).end(); itr2++)
            {
                if (*itr2 != '\n')
                    file << *itr2;
            }
            // Check if it's the last row before adding a newline
            if (next(itr) != text.end())
            {
                file << '\n';
            }
        }
        file.close();
    }
    void load(string filename = "test.txt")
    {
        ifstream reader;
        string line;
        reader.open(fileName);
        rowItr = text.begin();
        (*rowItr).pop_back(); // remove the end line character
        colItr = (*rowItr).begin();
        while (getline(reader, line))
        {
            // insert char by char in row iterator
            for (size_t ch = 0; ch < line.size(); ch++)
            {
                (*rowItr).insert(colItr, line[ch]);
            }

            (*rowItr).insert(colItr, '\n');                 // last character must be end line character
            text.push_back(list<char>());                   // create new row
            size_t offset = distance(text.begin(), rowItr); // current row iterator
            rowItr = text.begin();                          // to avoid any error
            advance(rowItr, offset + 1);                    // next row of current row
            colItr = (*rowItr).begin();
        }
        text.pop_back(); // last row is empty
        reader.close();
        updateItrToLast();
    }
    void updateItrToLast()
    {
        rowItr = text.begin();
        currentRow = text.size() - 1;
        advance(rowItr, currentRow);
        colItr = (*rowItr).begin();
        advance(colItr, (*rowItr).size() - 1); // pointing to the end line character '\n'
        currentCol = (*rowItr).size() - 1;
    }
    void printText()
    {
        system("cls");
        for (auto itr = text.begin(); itr != text.end(); itr++)
        {
            for (auto itr2 = (*itr).begin(); *itr2 != '\n'; itr2++)
            {
                cout << *itr2;
            }
            cout << '\n';
        }
        gotoRowColomn(currentRow, currentCol);
    }
    void printRow(list<list<char>>::iterator rowItr)
    {
        gotoRowColomn(currentRow, 0);
        for (auto colItr = rowItr->begin(); colItr != rowItr->end(); colItr++)
        {
            cout << *colItr;
        }
    }
    bool isValidInput(char ch)
    {
        return isupper(ch) || islower(ch) || isdigit(ch) || isspace(ch) || isSpecialCharacter(ch);
    }
    bool isSpecialCharacter(char ch)
    {
        const char specialChars[] = "`~!@#$%^&*()_+=-|}{\":?<>[]\\';/.,\"";

        for (char specialChar : specialChars)
        {
            if (ch == specialChar)
            {
                return true;
            }
        }
        return false;
    }
};
bool fileExist(string fileName)
{
    for (string file : files)
    {
        if (file == fileName)
        {
            return true;
        }
    }
    return false;
}
void readFiles(string fileName = "files.txt")
{
    ifstream file;

    file.open(fileName);
    string line;
    while (getline(file, line))
    {
        files.push_back(line);
    }
    file.close();
}
void writeFiles(string newFile, string fileName = "files.txt")
{
    ofstream file;
    file.open(fileName, ios::app);
    file << newFile << endl;
    file.close();
}
int main()
{
    system("color F0");
    textEditor editor = textEditor();
    readFiles();

    editor.load();
    editor.input();

    return 0;
}
