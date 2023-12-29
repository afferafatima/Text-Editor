#include <iostream>
#include <list>
#include <stack>
#include <deque>
#include <fstream>
#include <string>
#include <windows.h>
#include <conio.h>
#include <vector>
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
        state s;
        s.text.push_back(list<char>());
        auto r_itr = s.text.begin(); // iterator of state s
        s.row = currentRow;
        s.col = currentCol;
        s.colItr = colItr;
        s.rowItr = rowItr;
        // Iterate through each row in the currentFile's text(deep copy)
        for (auto row = text.begin(); row != text.end(); r_itr++, ++row)
        {
            // Iterate through each character in the row
            for (auto col = (*row).begin(); col != (*row).end(); ++col)
            {
                // Add a new character to the state's row
                (*r_itr).push_back(*col);
            }
            s.text.push_back(list<char>());
        }

        s.text.pop_back(); // remove the last empty row

        // Find the row in the state's text that corresponds to the currentRow
        s.rowItr = s.text.begin();
        advance(s.rowItr, currentRow); // loop

        // Find the column in the state's row that corresponds to the currentCol
        s.colItr = (*s.rowItr).begin();
        advance(s.colItr, currentCol); // loop
        // Set the row and column indices in the state

        return s;
    }
    // load given state (previous state)
    void loadState(state s)
    {
        // Clear existing text
        for (auto row = text.begin(); row != text.end(); ++row)
        {
            (*row).clear();
        }
        text.clear();

        // Initialize text with the loaded state's text
        text = s.text;

        // Set iterators and counters to the values from the loaded state
        currentRow = s.row;
        currentCol = s.col;
        rowItr = text.begin();
        advance(rowItr, currentRow);

        colItr = rowItr->begin();
        advance(colItr, currentCol);

        
    }
   // undo key
    void undoOperation()
    {
        if (undo.empty())
        {
            // cout << "No undo operation available" << endl;
            return;
        }

        state s = getState(); // Save the current state before applying undo

        redo.push(s); // Push the current state onto the redo stack

        s = undo.back(); // Get the state to undo
        undo.pop_back();

        loadState(s); // Load the state from undo stack
    }
    // redo key
    void redoOperation()
    {
        if (redo.empty())
        {
            // cout << "No redo operation available" << endl;
            return;
        }
        updateUndo();
        state s = redo.top();
        redo.pop();
        loadState(s);
    }
    // update undo stack
    void updateUndo()
    {
        if (undo.size() > 5)
        {
            undo.erase(undo.begin());
        }
        state s = getState();
        undo.push_back(s);
    }
    // delete key
    void deleteOperation()
    {
        updateUndo();
        if (currentCol == (*rowItr).size() - 1)
        {
            if (currentRow == text.size() - 1) // rowItr == text.end() not working
            {
                // cout << "No character to delete" << endl;
                return;
            }
            else
            {
                (*rowItr).pop_back(); // removes last endline character
                auto nextRowItr = next(rowItr);
                (*rowItr).splice((*rowItr).end(), *nextRowItr);
                text.erase(nextRowItr);
            }
        }
        else
        {
            (*rowItr).erase(colItr);
            colItr = (*rowItr).begin();
            advance(colItr, currentCol);
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
            colItr=(*rowItr).begin();
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

        auto nextRowItr = next(rowItr);
        list<char> newRow(colItr, (*rowItr).end());
        (*rowItr).erase(colItr, (*rowItr).end());
        (*rowItr).push_back('\n');
        text.insert(nextRowItr, newRow);

        // Update the iterators to point to the beginning of the next line

        currentRow++;
        rowItr = text.begin();
        advance(rowItr, currentRow);

        currentCol = 0;
        gotoRowColomn(currentRow, currentCol);
    }
    // backspace key
    void backSpaceOperation()
    {
        updateUndo();

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
    // main input function
    void input()
    {
        printText(text);
        char ch;
        while (true)
        {
            ch = _getch();
            if (ch == 26) // ctrl+z
            {

                undoOperation();
                printText(text);
            }
            else if (ch == 25) // ctrl+y
            {

                redoOperation();
                printText(text);
            }
            if (ch == 19) // ctrl+s
            {
                save();
            }
            else if (ch == 13) // enter key
            {

                newLineOperation();
                printText(text);
            }
            else if (ch == 8) // backspace
            {

                backSpaceOperation();
                printText(text);
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
                    printText(text);
                }
            }
            else
            {
                insertOperation(ch);
                printText(text);
            }
        }
    }
    // insert in notepad
    void insertOperation(char ch)
    {
        if (isValidInput(ch))
        {
            updateUndo();
            size_t offset = distance((*rowItr).begin(), colItr);

            (*rowItr).insert(colItr, ch);
            colItr = (*rowItr).begin();
            colItr = next(colItr, offset); // points to the same character that is inserted
            colItr++;
            currentCol++;
        }
    }
    // save file
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
    // load file
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
    //when program starts
    void updateItrToLast()
    {
        rowItr = text.begin();
        currentRow = text.size() - 1;
        advance(rowItr, currentRow);
        colItr = (*rowItr).begin();
        advance(colItr, (*rowItr).size() - 1); // pointing to the end line character '\n'
        currentCol = (*rowItr).size() - 1;
    }
    void printText(list<list<char>> text)
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
    // check if input is valid
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
void printBox(int x, int y, int gotox, int gotoy)
{
    // x rows
    // y coloms

    for (int i = 0; i < y; i++)
    {
        for (int j = 0; j < x; j++)
        {
            gotoRowColomn(gotox + j, gotoy + i);
            if (i == 0 || i == y - 1 || j == 0 || j == x - 1)
            {
                cout << "#";
            }
            else
            {
                cout << " ";
            }
        }
    }
}
string getFileName()
{
    printBox(5, 50, 10, 10);
    {
        gotoRowColomn(12, 12);
        cout << "Enter File Name(without .txt): ";
        string fileName;
        cin >> fileName;
        fileName += ".txt";
        return fileName;
    }
}
void deleteFile(string fileName)
{
    remove(fileName.c_str());
}
void makeFile(string fileName)
{
    ofstream file;
    file.open(fileName);
    file << '\n';
    file.close();
}

int main()
{
    system("color F0");
    system("cls");

    readFiles();
    fileName = getFileName();
    if (!fileExist(fileName))
    {
        writeFiles(fileName);
        makeFile(fileName);
    }
    textEditor editor = textEditor();
    editor.load(fileName);
    editor.input();

    return 0;
}
