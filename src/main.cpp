#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <unordered_map>

#define ERROR(content)\
        std::cerr << Hazel::RED << "Error: " << Hazel::YELLOW << __FILE__ << Hazel::RESET\
            << " : in function " << Hazel::CYAN <<  __func__ << Hazel::RESET\
            << " at line " << Hazel::MAGENTA << __LINE__ << Hazel::RESET << std::endl\
            << "        Compiled on " << __DATE__\
            << " at " << __TIME__ << std::endl\
            << "        " << Hazel::RED << content << Hazel::RESET << std::endl;

namespace Hazel
{
    // ��ɫ����
    const char* RESET = "\033[0m";
    const char* BLACK = "\033[30m";
    const char* RED = "\033[31m";
    const char* GREEN = "\033[32m";
    const char* YELLOW = "\033[33m";
    const char* BLUE = "\033[34m";
    const char* MAGENTA = "\033[35m";
    const char* CYAN = "\033[36m";
    // һЩ��С����
    const int ACTIVEMAXLEN = 100;  // �����Ĵ�С
    const int MAXLINESIZE = 320;  // ÿһ�е���󳤶�
    const int MAXBLOCKSIZE = 81;  // ÿһ���п����󳤶�
    // �ַ���ƥ���㷨��ѡ��
    const int KMP = 1;
    const int TRIE = 2;
    const int BF = 3;
};

using namespace std;
using namespace Hazel;

// �ṹ��Ķ���

// �洢ÿһ�е��ı�����ʽ�洢
typedef struct LineBlock {
    char data[MAXBLOCKSIZE];  // ����ÿһ����ַ�����
    LineBlock* next;  // ָ����һ���п�
    // Ĭ�Ϲ���
    LineBlock(const char* text, LineBlock* ne = nullptr)
    {
        int length = strlen(text);
        memcpy(data, text, min(length, MAXLINESIZE - 1));  // ��text�����ݿ�����data
        data[length] = '\0';
        next = ne;
    }
} LineBlock;
// �洢�����е�ÿһ�У���ʽ�洢
typedef struct Line {
    int line_no;  // �к�
    LineBlock* content;  // ָ��������
    Line* next;  // ָ����һ��
    Line(int number, LineBlock* line_block, Line* ne = nullptr) : line_no(number), content(line_block), next(ne) {}
} Line, *ActiveArea;
// �洢������ƥ�䵽���ַ���������λ��(row,col)
typedef struct Position {
    int row, col;
} Pos;
// Trie�ڵ�
struct TrieNode {
    bool isEnd;  // ����Ƿ��ǵ��ʵĽ�β
    std::unordered_map<char, TrieNode*> son;  // �ӽڵ㼯��
    TrieNode() : isEnd(false) {}
};
// Trie��
class Trie {
public:
    // Ĭ�Ϲ���
    Trie() : root(new TrieNode()) {}
    // �����ַ���
    void insert(const char* word)
    {
        TrieNode* node = root;
        while (*word)
        {
            if (node->son.find(*word) == node->son.end())
                node->son[*word] = new TrieNode();
            node = node->son[*word];
            word ++ ;
        }
        node->isEnd = true;
    }
    // �����ַ���
    bool search(const char* src, const char* match_str, int pos[], int& cnt)
    {
        cnt = 0;
        insert(match_str);
        int len = strlen(src);
        for (int i = 0; i < len; i ++ )
        {
            TrieNode* node = this->root;
            for (int j = i; j < len; j ++ )
            {
                if (node->son.find(src[j]) == node->son.end()) break;
                node = node->son[src[j]];
                // ƥ��ɹ�������λ��
                if (node->isEnd) pos[cnt ++ ] = i;
            }
        }
        return cnt > 0;
    }

private:
    TrieNode* root;
};


// ����������

// ��ʾ����
void showMainMenu();
void showHelpMenu();
void showActiveArea(const ActiveArea&, int, int, int);
void printLine(const LineBlock*);
void printPositions(Pos[], int);
// ��������
void readFile(char*, char*);
int getValidInput();
void getNumber(const char*, int&, int, int);
void getString(const char*, char*, int, int);
bool CHECK_AREA(const ActiveArea&);
bool CHECK_BLOCK(const LineBlock*);
bool CHECK_BOUND(int, int, int, int low = 0);
// ��Line������һЩ����
bool initLine(Line*&);
int countLine(const ActiveArea&);
bool emptyArea(const ActiveArea&);
bool insertLine(ActiveArea&, char*, int, ActiveArea& otherArea, char* output_file = nullptr);
void changeActiveArea(ActiveArea&, ActiveArea&, char*);
bool deleteLine(ActiveArea&, ActiveArea&, int, int);
bool matchString(ActiveArea&, char*, int, int, Pos[], int&, int);
bool replaceString(ActiveArea&, int, char*, char*, int&, int);
// ��LineBlock������һЩ����
bool insertLineBlock(LineBlock*&, char*);
bool emptyLineBlock(const LineBlock*);
bool clearLineBlock(LineBlock*&);
void blocks_to_str(LineBlock*, char*);
void str_to_blocks(char*, LineBlock*&);
// ���ļ�������һЩ����
bool readFromInputFile(char*, ActiveArea&, ActiveArea&, char*);
bool writeToOutputFile(char*, ActiveArea&, ActiveArea&, int, int);
bool readFromOtherArea(ActiveArea&, ActiveArea&, int);
// ���ַ���������һЩ����
bool insertString(char*, int, const char*);
bool eraseString(char*, int, int);
bool kmp(char*, char*, int[], int&);
bool trie(char*, char*, int[], int&);
bool bf(char*, char*, int[], int&);


// �����ľ���ʵ��

/* 1. ��ʾ������ʵ�� */
// ��ʾ����������
void showMainMenu()
{
    cout << Hazel::GREEN << "******  Welcome to Hazel Editor  ******" << Hazel::RESET << endl;
    cout << "If you are new, you can type help to get help" << endl;
}
// ��ʾ�����˵�
void showHelpMenu()
{
    cout << Hazel::GREEN << "help menu" << Hazel::RESET << endl;
    cout << Hazel::CYAN << "********************************************************************************" << Hazel::RESET << endl;
    cout << Hazel::CYAN << "*" << Hazel::RESET << " 1. Insert Line: Format - i<line_number><Enter><text><Enter>. Inserts <text> after line <line_number> in the active area.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 2. Delete Line: Format - d<line_number1>[ <line_number2>]<Enter>. Deletes line <line_number1> (to line <line_number2>) in the active area.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 3. Switch Active Area: Format - n<Enter>\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 4. Display Active Area: Format - p<Enter>\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 5. Replace String: Format - S<line_number>@<string1>@<string2><Enter>. Replaces <string1> with <string2> in line <line_number> of the active area.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 6. Match String: Format - m<string><Enter>. Matches all occurrences of <string> in the active area.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 7. Read From File: Format - r <file_name><enter>.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 8. Write To File: Format - w <file_name><enter>.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 9. Exit: Format - exit" << endl;
    cout << Hazel::CYAN << "********************************************************************************" << Hazel::RESET << endl;
}
// ��ʾ�������ӵ�begin�п�ʼ��ʾ��һ����ʾcnt��
void showActiveArea(const ActiveArea& activeArea, int begin, int cnt, int page_num)
{
    cout << Hazel::GREEN << "---------------  Current Active Area (Page " << page_num << ")  ---------------" << Hazel::RESET << endl;
    if (emptyArea(activeArea))
    {
        cout << Hazel::YELLOW << setw(4) << "Current Active Area is None!\n" << Hazel::RESET << endl;
        return;
    }
    // �����ǰ����
    Line* cur = activeArea;
    while (cur && begin -- ) cur = cur->next;  // �ҳ�Ҫ��ʾ�Ļ�����ʼ�е�begin��
    if (!cur)  // ��curΪ�գ�˵������������β��
    {
        cout << Hazel::YELLOW << setw(4) << "Current Active Area is None!\n" << Hazel::RESET << endl;
        return;
    }
    // ��ʱcurָ���begin��
    while (cur && cnt -- )
    {
        cout << Hazel::BLUE << std::setw(4) << cur->line_no << Hazel::RESET << ' ';  // ��ʾ�кţ���ɫ��ʾ��
        printLine(cur->content);
        cur = cur->next;
    }
    cout << Hazel::YELLOW << setw(60) << "total line: " << activeArea->line_no << Hazel::RESET << endl;
    cout << endl;
}
// ��ӡһ�е��ı�����
void printLine(const LineBlock* line_content)
{
    while (line_content)
    {
        cout << line_content->data;
        line_content = line_content->next;
    }
    cout << endl;
}
// ��ӡλ��(row,col)����
void printPositions(Pos positions[], int n)
{
    for (int i = 0; i < n; i ++ )
        cout << i + 1 << "-th match position: " << Hazel::CYAN <<positions[i].row << ", " << positions[i].col << Hazel::RESET << endl;
}


/* 2. ����������ʵ�� */
// ��ȡ��������ļ���
void readFile(char* input_file, char* output_file)
{
    cout << "Please enter Input File: ";
    cin >> input_file;
    cout << "Please enter Output File: ";
    cin >> output_file;
}
// ��ȡ��Ч����
int getValidInput()
{
    int input;
    while (true)
    {
        cin >> input;
        if (cin.fail())  // ���������Ч��
        {
            cout << Hazel::RED << "Invalid Input! Please input a number" << Hazel::RESET << endl;
            cin.clear();  // �������״̬
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ������Ч������
        }
        else break;
    }
    return input;
}
// ��ȡ��src�д��±�Ϊ[begin,end]�������ڵ����ָ�res ����end������Χ��ʾ�����ַ���������
void getNumber(const char* src, int& res, int begin, int end)
{
    res = 0;
    for (int i = begin; src[i] && i <= end; i ++ ) res = res * 10 + (src[i] - '0');
}
// ��ȡsrc���ִ�src[begin,end]��dist
void getString(const char* src, char* dist, int begin, int end)
{
    int cnt = 0;
    for (int i = begin; i <= end; i ++ ) dist[cnt ++ ] = src[i];
    dist[cnt] = '\0';
}
// ���ActiveArea�Ƿ�Ϸ�
bool CHECK_AREA(const ActiveArea& area)
{
    if (emptyArea(area))
    {
        ERROR("The area is not exist")
        return false;
    }
    return true;
}
// ����п��Ƿ�Ϸ�
bool CHECK_BLOCK(const LineBlock* block)
{
    if (emptyLineBlock(block))
    {
        ERROR("The block is not exist")
        return false;
    }
    return true;
}
// �������[begin,end]�Ƿ�Ϸ���highΪ������Ͻ磬lowΪ�½磨Ĭ��Ϊ0��
bool CHECK_BOUND(int begin, int end, int high, int low)
{  // Ҳ���Ըĳ��׳��쳣
    if (begin < low || begin > high)
    {
        ERROR("The parameter 'begin' is invalid")
        // throw std::runtime_error("index out of bounds");
        return false;
    }
    if (end < low || end > high)
    {
        ERROR("The parameter 'end' is invalid")
        // throw std::runtime_error("index out of bounds");
        return false;
    }
    if (begin > end)
    {
        ERROR("Invalid interval")
        // throw std::runtime_error("invalid interval");
        return false;
    }
    return true;
}
// ��������
void clearScreen()
{
    #ifdef _WIN32
    system("cls");
    #elif __linux__
    system("clear");
    #endif
}

/* 3. ��Line��������غ�����ʵ�� */
// ��ʼ��Line*(ActiveArea)
bool initLine(Line*& line)
{
    try
    {
        line = new Line(0, nullptr, nullptr);
    }
    catch (std::bad_alloc& e)
    {
        ERROR(e.what())
        return false;
    }
    return true;
}
// ���ػ����ĳ��ȣ���������
int countLine(const ActiveArea& area)
{
    if (emptyArea(area)) return 0;
    return area->line_no;
}
// �ж�ActiveArea�Ƿ�Ϊ��
bool emptyArea(const ActiveArea& area)
{
    return area == nullptr || area->next == nullptr;
}
// ���в��뵽��i�к��棬��output_fileΪ��ʱ˵���������ǻ����в�������
bool insertLine(ActiveArea& activeArea, char* text, int i, ActiveArea& otherArea, char* output_file)
{
    if (i < 0 || i + 1 > ACTIVEMAXLEN)  // ����λ�ò��Ϸ�
    {
        ERROR("The position of the inserted row is invalid")
        return false;
    }
    // 1. ���������ı�
    LineBlock* line_content = nullptr;  // ���е�ͷָ��
    str_to_blocks(text, line_content);  // ���ַ���ת���ɿ����ʽ�洢��line_content��
    // 2. ���ı����������
    Line* new_line = new Line(i + 1, line_content);  // ���У���Ϊ��i+1�д����������
    if (emptyArea(activeArea))  // �������в������κ��У�����������
    {
        initLine(activeArea);
        activeArea->next = new_line;
        return true;
    }
    Line* cur = activeArea;  // curָ�����ڵ��ǰ���ڵ㣬��ʼ��Ϊͷ�ڵ�
    while (cur->next && i -- ) cur = cur->next;  // Ѱ�Ҳ���λ��
    if (i > 0)
    {
        ERROR("The position of the inserted row is invalid")
        return false;
    }
    new_line->next = cur->next;
    cur->next = new_line;
    // �޸�new_line���������е��к�
    cur = new_line->next;
    while (cur)
    {
        cur->line_no ++ ;
        cur = cur->next;
    }
    activeArea->line_no ++ ;  // ���»�������ǻ���������
    // ֻ��д�����ʱ����Ҫ��飺���������к�������ȳ������ƣ��򽫵�һ�������ֻ��output_file��ֵʱ�����ǲ��뵽�����У�
    if (activeArea->line_no > ACTIVEMAXLEN && output_file)
        writeToOutputFile(output_file, activeArea, otherArea, 1, 1);  // �������еĵ�1�����
    return true;
}
// �����л���������д������ļ������ӷǻ����ж�ȡ��һ�Σ���Ϊ�µĻ�����
void changeActiveArea(ActiveArea& activeArea, ActiveArea& otherArea, char* output_file)
{
    if (!CHECK_AREA(activeArea)) return;
    int len = countLine(activeArea);  // len�洢Ҫ�ѻ����ж����е�����д������ļ���Ĭ��Ϊȫ��
    // �������ļ���δ���꣬�����л�����ɽ�ԭ���������10�����ڻ����������Ա����Ķ������ԣ���������ζ�Ž����༭��ʼ�༭��һ���ļ�
    if (countLine(otherArea)) len -= 10;
    writeToOutputFile(output_file, activeArea, otherArea, 1, len);
}
// ɾ��������[begin,end]����������У����ǻ����л��������ݣ��򽫷ǻ����е����ݽӵ����������һ�к���
bool deleteLine(ActiveArea& activeArea, ActiveArea& otherArea, int begin, int end)
{
    if (!CHECK_AREA(activeArea)) return false;
    if (!CHECK_BOUND(begin, end, countLine(activeArea), 1)) return false;
    // ɾ��[begin,end]�ڵ�������
    int len = end - begin + 1;  // len�洢ɾ����������
    Line* cur = activeArea;
    while (cur->next && -- begin) cur = cur->next;  // ��cur�ҵ���begin�е�ǰһ��
    Line* left = cur;  // leftָ���begin�е�ǰһ��
    // ɾ��[begin,end]�ڵ������У�����curָ���end�еĺ�һ��
    int tmp_length = len;  // ��ʱ�����洢len
    cur = cur->next;  // ��ʱcurָ���beg��
    while (cur && tmp_length -- )  // ����ɾ��len��
    {
        Line* tmp = cur;
        cur = cur->next;
        delete tmp;
    } // ��ʱcurָ���end�еĺ�һ��
    left->next = cur;
    // ����end�к��������е��к�
    while (cur)
    {
        cur->line_no -= len;
        cur = cur->next;
    }
    // ��������
    activeArea->line_no -= len;
    // ���ǻ����л������ݣ�����뵽�����У������������ɾ����������ͬ
    if (!emptyArea(otherArea)) readFromOtherArea(activeArea, otherArea, len);
    return true;
}
// �ڻ���activeArea�е�[begin,end]��������ƥ���ַ���match_str�����ѽ������Pos�����У�cntΪPos���鳤�ȣ�typeָ��ƥ���㷨��Ĭ��ʹ��KMP�㷨����ƥ�䡣��ƥ��ɹ�������true
bool matchString(ActiveArea& activeArea, char* match_str, int begin, int end, Pos positions[], int& cnt, int type = Hazel::KMP)
{
    if (!CHECK_AREA(activeArea)) return false;
    if (!CHECK_BOUND(begin, end, countLine(activeArea))) return false;
    cnt = 0;  // cnt��ʼ����0
    Line* cur = activeArea;
    while (cur->next && begin -- ) cur = cur->next;  // �ҵ���begin�У�����cntָ���begin��
    int len = end - begin + 1;  // len�洢�ܹ��ж�������Ҫƥ��
    while (cur->next && len -- )
    {
        char line_content[MAXLINESIZE];  // �洢��ǰ�е��ı�����
        blocks_to_str(cur->content, line_content);  // ����ǰ�п�ת��һ������ַ�����
        // �ڵ�ǰ���ڽ���ƥ�䣨kmp��
        int pos[MAXLINESIZE] = {0}, matched_cnt = 0;  // pos��λ���±��0��ʼ������±���Ҫ+1
        // ָ��ƥ���㷨
        if (type == Hazel::KMP) kmp(line_content, match_str, pos, matched_cnt);
        else if (type == Hazel::TRIE) trie(line_content, match_str, pos, matched_cnt);
        else if (type == Hazel::BF) bf(line_content, match_str, pos, matched_cnt);
        else kmp(line_content, match_str, pos, matched_cnt);
        // ��ƥ��ɹ����ַ���λ�ô���positions��
        for (int i = 0; i < matched_cnt; i ++ ) positions[cnt ++ ] = {cur->line_no, pos[i] + 1};
        // ����������һ��ƥ��
        cur = cur->next;
    }
    return cnt > 0;  // ��cnt����0��˵��ƥ�䵽�ˣ�����û��ƥ�䵽
}
// �������е�line_no�е��ַ���original�滻���ַ���replaced��typeָ��ƥ���㷨��Ĭ��ʹ��KMP�㷨
bool replaceString(ActiveArea& activeArea, int line_no, char* original, char* replaced, int& cnt, int type = Hazel::KMP)
{
    if (!CHECK_AREA(activeArea)) return false;
    cnt = 0;  // ��ʼ�����滻����cntΪ0
    int tmp = line_no;  // ��ʱ������¼Ŀ���к�
    char line_content[MAXLINESIZE];  // �洢��ǰ�������ı�����
    Line* cur = activeArea;
    while (cur->next && tmp -- ) cur = cur->next;  // �ҵ���line_no��
    blocks_to_str(cur->content, line_content);  // ����line_no�е�ÿһ��ϲ���һ���������ַ�����ֵ��line_content
    // �ַ���ƥ��
    int matched[MAXLINESIZE] = {0};  // matched�洢ƥ��ɹ����±�
    // ָ��ƥ���㷨
    if (type == Hazel::KMP) kmp(line_content, original, matched, cnt);
    else if (type == Hazel::TRIE) trie(line_content, original, matched, cnt);
    else if (type == Hazel::BF) bf(line_content, original, matched, cnt);
    else kmp(line_content, original, matched, cnt);
    // ö��ÿ���滻��
    int original_length = strlen(original);
    int replaced_length = strlen(replaced);
    int last = 0;  // last�洢�滻���´��������ַ�����ƫ����
    for (int i = 0; i < cnt; i ++ )
    {
        int idx = matched[i] + last;  // ��ȡƥ�䵽��ÿ���ַ���ĸ��line_content�е�����±꣬��Ҫ����ƫ����
        // idxʼ��ָ��ƥ�䵽���ַ�����line_content�е��±�
        if (idx > MAXLINESIZE) break;  // ��������Χ��ֱ���˳����滻����
        eraseString(line_content, idx, idx + original_length - 1);  // ɾ���ַ���line_content�е�original�ַ���
        insertString(line_content, idx, replaced);  // �ڵ�idx���ַ���������ַ���replaced
        last += replaced_length - original_length;  // ����ƫ����last
    }
    str_to_blocks(line_content, cur->content);  // ���޸ĺ��line_content��������cur->content
    // !ע�⣺������ʱ����LineBlock* content��¼cur->content����Ѳ�����Ľ��ӳ�䵽activeArea��
    return cnt > 0;
}

/* 4. LineBlock��غ�����ʵ�� */
// ���п���뵽line_block���棬β�巨
bool insertLineBlock(LineBlock*& line_block, char* text)  
{
    if (!line_block)  // ������Ϊ�գ���ֱ�Ӳ���
    {
        line_block = new LineBlock(text);
        return true;
    }
    // ���뵽�е�β������Ϊ��ֻ��һ���б༭����
    LineBlock* cur = line_block;
    while (cur->next) cur = cur->next;
    LineBlock* new_block = new LineBlock(text);
    cur->next = new_block;
    return true;
}
// �ж�LineBlock�Ƿ�Ϊ��
bool emptyLineBlock(const LineBlock* block)
{
    return block == nullptr;
}
// ���LineBlock
bool clearLineBlock(LineBlock*& blocks)
{
    if (!CHECK_BLOCK(blocks)) return false;
    // ���ÿһ���ַ�����
    while (blocks)
    {
        LineBlock* tmp = blocks;  // ��ʱָ�룬ָ��Ҫɾ�����Ǹ��ڵ�
        blocks = blocks->next;  // ͷָ�����
        delete tmp;  // ɾ���ڵ�
    }
    blocks = nullptr;  // �ÿ�
    return true;
}
// ��src�е�ÿһ�鿽�����ַ�����dist
void blocks_to_str(LineBlock* src, char* dist)
{
    LineBlock* cur = src;
    // ��ÿһ�鿽����dist
    int len = 0;  // len�洢dist�ַ����鳤��
    while (cur)
    {
        for (int i = 0; cur->data[i]; i ++ ) dist[len ++ ] = cur->data[i];  // ���ַ�����
        cur = cur->next;
    }
    dist[len] = '\0';  // ����ַ�������
}
// ���ַ�����src����80���ַ�һ�鿽����dist
void str_to_blocks(char* src, LineBlock*& dist)
{
    if (!emptyLineBlock(dist)) clearLineBlock(dist);  // �����dist
    // �ٽ�src�ֿ�
    int len = strlen(src);  // len�洢��ǰ��ʣ�¶����ַ�û����
    char block_content[MAXBLOCKSIZE];  // �洢ÿһ����ı�
    int idx = 0;  // ��¼src���±�����
    while (len > 0)  // ���δ���ÿһ��
    {
        int block_len = 0;  // ��¼��ǰ��Ĵ�С
        for (int i = idx; src[i]; i ++ )  // ��src�е����ݿ�����ÿһ����
        {
            if (block_len == MAXBLOCKSIZE - 1) break;
            block_content[block_len ++ ] = src[i];
        }
        block_content[block_len] = '\0';  // �����һ���ַ���Ϊ'\0'����ʾ�����
        insertLineBlock(dist, block_content);  // ���ÿ����ݲ��뵽dist��
        len -= MAXBLOCKSIZE - 1;  // ����len
    }
}

/* 5. ���ļ�������һЩ���� */
// �������ļ��е����ݶ�������У���ȡʧ�ܷ���false
bool readFromInputFile(char* input_file, ActiveArea& activeArea, ActiveArea& otherArea, char* output_file)
{
    std::ifstream ifs(input_file);
    if (!ifs.is_open())
    {
        ERROR("Input file opening failed")
        return false;
    }
    cout << "Reading from the file: '" << Hazel::MAGENTA << input_file << Hazel::RESET <<  "' ...\n";
    char text[MAXLINESIZE];
    int cnt = 0;  // ����
    while (ifs.getline(text, MAXLINESIZE) && cnt < ACTIVEMAXLEN - 20)  // ����ȡACTIVEMAXLEN - 20��
        insertLine(activeArea, text, cnt ++ , otherArea, output_file);  // ��text���뵽��cnt�к���
    activeArea->line_no = cnt;  // ���»����ĳ���
    // ��ʣ�ಿ�ֶ���ǻ������ǻ����޳�������
    cnt = 0;
    while (ifs.getline(text, MAXLINESIZE)) insertLine(otherArea, text, cnt, activeArea, nullptr);  // ʣ�ಿ�ֶ���ǻ���
    ifs.close();  // �ر��ļ�������
    return true;
}
// ������������[begin, end]�е�����д������ļ�
bool writeToOutputFile(char* output_file, ActiveArea& activeArea, ActiveArea& otherArea, int begin, int end)
{
    std::ofstream ofs(output_file, ios::app);  // ��׷�ӵķ�ʽ���ļ�
    if (!ofs.is_open())
    {
        ERROR("Output file opening failed")
        return false;
    }
    if (!CHECK_BOUND(begin, end, countLine(activeArea))) return false;

    // �������ڵ�������д������ļ���
    cout << "Writing to the file: '" << Hazel::MAGENTA << output_file << Hazel::RESET <<"' ...\n";
    int len = end - begin + 1;  // len�洢Ҫд������ļ���������
    char text[MAXBLOCKSIZE];
    Line* cur = activeArea;
    while (cur->next && -- begin) cur = cur->next;  // �ҵ���begin�е�ǰ���ڵ�
    Line* left = cur;  // leftָ���begin�е�ǰ��
    cur = cur->next;  // ��curָ���begin��
    for (int i = 0; i < len; i ++ )  // ����д���begin�е���end��
    {
        Line* tmp = cur;
        char text[MAXLINESIZE];
        blocks_to_str(cur->content, text);
        ofs << text << endl;
        cur = cur->next;  // ������һ��
        delete tmp;
    }
    left->next = cur;  // ����left�к�end��һ����������
    // ���ǻ����л������ݣ���ӷǻ����ж���len�е�������
    if (!emptyArea(otherArea)) readFromOtherArea(activeArea, otherArea, len);
    ofs.close();  // �ر�����ļ���
    return true;
}
// ���ǻ����е�len��д������� ��������len�У���ȫ��д�������
bool readFromOtherArea(ActiveArea& activeArea, ActiveArea& otherArea, int len)
{
    if (!CHECK_AREA(activeArea) || !CHECK_AREA(otherArea)) return false;
    int actualLength = 0;  // �洢ʵ��д�������
    Line* tail = activeArea->next;  // tailָ�������β�ڵ�
    while (tail) tail = tail->next;
    // ��otherArea��ǰlen�ж���activeArea
    Line* head = otherArea->next;  // headָ��ǻ�������Ԫ�ڵ�
    while (head && len -- )
    {
        tail->next = head;
        otherArea->next = head->next;
        head = otherArea->next;  // ����headʼ��ָ��ǻ�������Ԫ�ڵ�
        tail = tail->next;  // tail����
        tail->next = nullptr;  // tail��nextָ���ÿ�
        actualLength ++ ;  // ʵ��д������+1
    }
    // ���»����ͷǻ����ĳ���
    otherArea->line_no -= actualLength;
    activeArea->line_no += actualLength;
    return true;
}


/* 6. �ַ�����غ�����ʵ�� */
// ��str�е�pos���ַ��ĺ�������ַ���value
bool insertString(char* str, int pos, const char* value)
{
    int n = strlen(str);
    if (pos < 0 || pos > n)  // ����λ�ò��Ϸ�
    {
        ERROR("The parameter 'pos' is invalid")
        return false;
    }
    int len = strlen(value);  // len��ʾҪ������ַ����ĳ���
    // �Ƚ�ԭ������λ�ú���������ַ����ƫ��lenλ��Ϊvalue�Ĳ����ڳ��ռ���
    for (int i = n + len; i > pos + len - 1; i -- )
    {
        if (i >= MAXLINESIZE - 1) continue;  // ���������Χ������
        str[i] = str[i - len];
    }
    // �ٰ�Ҫ��ӵ��ַ���value�������ڳ����ǿ�ռ���
    for (int i = pos; i < pos + len; i ++ ) str[i] = value[i - pos];
    // ��������ַ�����������
    str[n + len] = '\0';
    return true;
}
// ɾ���ַ���str��[begin,end]�����ڵ��ַ�
bool eraseString(char* str, int begin, int end)
{
    int n = strlen(str);
    if (!CHECK_BOUND(begin, end, n)) return false;
    
    int len = end - begin + 1;  // �洢Ҫɾ�����ַ�����
    for (int i = end + 1; str[i]; i ++ ) str[i - len] = str[i];  // ��������ַ���ǰ��len����λ
    str[n - len] = '\0';  // ����ַ�������
    return true;
}
// kmp��O(n)��ƥ��ĸ��str�е�����ģʽ��match_str��������λ�÷���pos�����У�����Ϊcnt����ƥ��ɹ�������true
bool kmp(char* str, char* match_str, int pos[], int& cnt)
{
    cnt = 0;  // ��ʼ��cntΪ0
    int n = strlen(match_str), m = strlen(str);  // nΪģʽ�����ȣ�mΪĸ������
    char p[MAXLINESIZE + 1], s[MAXLINESIZE + 1];  // p��ģʽ����s��ĸ����ƫ����+1�����ü���
    memcpy(p + 1, match_str, min(n, MAXLINESIZE - 1));
    memcpy(s + 1, str, min(m, MAXLINESIZE - 1));
    p[n + 1] = s[m + 1] = '\0';  // ��ǽ�����
    // ��next����
    int ne[MAXLINESIZE + 1] = {0};
    for (int i = 2, j = 0; i <= n; i ++ )
    { // ne[1] = 0����Ϊ�����һ��ƥ�䲻�ϣ���ֻ�ܴ�0��ʼ����ƥ����
        while (j && p[i] != p[j + 1]) j = ne[j]; // ��KMPƥ���������
        if (p[i] == p[j + 1]) j ++ ;
        ne[i] = j; // ��¼ne[i]��ֵΪj
    }
    // KMPƥ�����
    for (int i = 1, j = 0; i <= m; i ++ )
    {
        while (j && s[i] != p[j + 1]) j = ne[j]; // ���s[i]��p[j + 1]����ƥ�䣬��j�ƶ���ne[j]��λ�ü�����s[i]ƥ�䣬ֱ��jΪ0
        if (s[i] == p[j + 1]) j ++ ; // ƥ��ɹ���j����ƶ�
        if (j == n) // ��ʱ�������ַ�����ƥ��ɹ�
        {
            pos[cnt ++ ] = i - n;  // ��ƥ��ɹ���λ�ô洢pos�����У��±��0��ʼ
            j = ne[j]; // ��ne[j]��ʼ��������һ��ƥ��
        }
    }
    return cnt > 0;
}
// trie��O(n)��ƥ��ĸ��str�е�����ģʽ��match_str��������λ�÷���pos�����У�����Ϊcnt����ƥ��ɹ�������true
bool trie(char* src, char* match_str, int pos[], int& cnt)
{
    Trie tr;
    return tr.search(src, match_str, pos, cnt);
}
// bf��O(nm)��ƥ��ĸ��str�е�����ģʽ��match_str��������λ�÷���pos�����У�����Ϊcnt����ƥ��ɹ�������true
bool bf(char* str, char* match_str, int pos[], int& cnt)
{
    cnt = 0;
    int str_len = strlen(str), match_len = strlen(match_str);
    for (int i = 0; i <= str_len - match_len; i ++ ) {
        int j;
        for (j = 0; j < match_len; j ++ )
            if (str[i + j] != match_str[j])
                break;
        // ƥ��ɹ�������λ��
        if (j == match_len) pos[cnt ++ ] = i;
    }
    return cnt > 0;
}

int main()
{
    ActiveArea activeArea = nullptr, otherArea = nullptr;

    char input_file[100], output_file[100];
    readFile(input_file, output_file);
    
    // ���������ļ��������ó���������ļ��ж�ȡ�ı�
    if (input_file)
    {
        readFromInputFile(input_file, activeArea, otherArea, output_file);
    }
    // ��ʼ�������
    char op[MAXLINESIZE] = {0};
    int count = 0;  // count��¼��ǰ�Ѿ����ж��ٴβ���
    while (true)
    {
        // ��ӡ�ϴβ���
        cout << Hazel::MAGENTA <<"last operation: " << (strlen(op) == 0 ? "none" : op) << Hazel::RESET << endl;
        // ��ʾ����
        showActiveArea(activeArea, 1, countLine(activeArea), 1);
        // չʾ����������
        showMainMenu();
        // ���뱾�β���
        cin.getline(op, sizeof op);
        // ��������+1
        count ++ ;

        // �жϲ�������
        // 1. �˳�ϵͳ
        if (strcmp(op, "exit") == 0)
        {
            break;
        }
        // 2. ��ʾ�����˵�
        else if (strcmp(op, "help") == 0)
        {
            showHelpMenu();
        }
        // 3. �в���
        else if (op[0] == 'i')
        {
            char str[MAXLINESIZE];
            cin.getline(str, sizeof str);
            int idx = 0;
            getNumber(op, idx, 1, strlen(op) - 1);  // ���뵽��idx��
            insertLine(activeArea, str, idx, otherArea, output_file);  // ���ò��뺯��
        }
        // 4. ɾ����
        else if (op[0] == 'd')
        {
            int i;
            for (i = 1; op[i] && op[i] != ' '; i ++ );  // ��iָ��ո�λ��
            int begin = 0, end = 0;  // ɾ���е����½�
            getNumber(op, begin, 1, i - 1);
            if (op[i] && op[i] == ' ') getNumber(op, end, i + 1, strlen(op) - 1);
            else end = begin;
            // ����ɾ���������ɾ������
            if (!emptyArea(activeArea)) deleteLine(activeArea, otherArea, begin, end);
        }
        // 5. �����л�
        else if (op[0] == 'n' && strlen(op) == 1)
        {
            changeActiveArea(activeArea, otherArea, output_file);  // ���û����л�����
        }
        // 6. ������ʾ
        else if (op[0] == 'p' && strlen(op) == 1/* || count == 3*/)
        {
            int cur_line = 1, show_lines = 20;  // ��ʼ��ʾ������һ��չʾ������
            int page_num = 1;  // �洢��ǰҳ��
            while (true)
            {
                showActiveArea(activeArea, cur_line, show_lines, page_num ++ );  // ��ʾһ�κ�ҳ��+1
                cur_line += show_lines;
                if (cur_line > countLine(activeArea))  // ��ʾ��ϣ��˳�
                {
                    cout << Hazel::YELLOW << "End of Active Area" << Hazel::RESET << endl;
                    break;
                }
                cout << "Do you want to continue to the next page? (Y/N): ";
                char choice; cin >> choice;  // ����ѡ��
                if (choice != 'Y') break;  // ����������ʾ������ʾ��ϣ��˳�
            }
        }
        // 7. �ַ����滻
        else if (op[0] == 'S')
        {
            int i, j;
            for (i = 1; op[i] != '@'; i ++ );  // ��iָ���һ��'@'��
            for (j = i + 1; op[j] != '@'; j ++ );  // ��jָ��ڶ���'@'��
            int line_no = 0;
            char original[MAXLINESIZE], replaced[MAXLINESIZE];
            getNumber(op, line_no, 1, i - 1);  // ��ȡ�к�
            getString(op, original, i + 1, j - 1);  // ��ȡԭ��
            getString(op, replaced, j + 1, strlen(op) - 1); // ��ȡ�滻��
            int replaced_cnt = 0;  // �洢�滻����
            replaceString(activeArea, line_no, original, replaced, replaced_cnt);  // �����滻�ַ�������
            cout << Hazel::CYAN << "total replaced counts: " << replaced_cnt << endl
                << "replaced successfully" << Hazel::RESET << endl;
        }
        // 8. �ַ���ƥ��
        else if (op[0] == 'm')
        {
            char match_str[MAXLINESIZE];
            getString(op, match_str, 1, strlen(op) - 1);  // ������ƥ���ַ���
            Pos positions[MAXLINESIZE * ACTIVEMAXLEN] = {0};  // �洢ƥ��ɹ���λ��
            int matched_cnt = 0;  // ��¼ƥ��ɹ��ĸ���
            cout << "total matches: " << matched_cnt << endl;
            if (matchString(activeArea, match_str, 1, countLine(activeArea), positions, matched_cnt))  // ƥ�����������match
                printPositions(positions, matched_cnt);  // ��ƥ��ɹ������ӡ����ƥ�䵽��λ��
        }
        // 9. �Ƿ�����
        else
        {
            if (count == 1)  // ��һ�β��ǷǷ�����
            {
                clearScreen();
                continue;
            }
            cout << Hazel::RED << "Invalid Input!" << Hazel::RESET << endl;
        }
        cout << Hazel::GREEN << "Press enter to continue..." << Hazel::RESET;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // �ж�ֻ���ջس�
        clearScreen();  // ÿ����һ�κ�����
    }

    return 0;
}