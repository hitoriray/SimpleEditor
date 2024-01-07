#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <unordered_map>


/* �궨�壬���ڴ�ӡ���ֲ�ͬ�ȼ�����Ϣ�����ڵ��� */
#define INFO(message) std::cout << Hazel::GREEN << message << Hazel::RESET << std::endl;
#define WARNINGS(message) std::cout << Hazel::YELLOW << message << Hazel::RESET << std::endl;
#define TRACE(message) std::cout << Hazel::MAGENTA << message << Hazel::RESET << std::endl;
#define ERROR(message)\
        std::cerr << Hazel::RED << "Error: " << Hazel::YELLOW << __FILE__ << Hazel::RESET\
            << " : in function " << Hazel::CYAN <<  __func__ << Hazel::RESET\
            << " at line " << Hazel::MAGENTA << __LINE__ << Hazel::RESET << std::endl\
            << "        Compiled on " << __DATE__\
            << " at " << __TIME__ << std::endl\
            << "        " << Hazel::RED << message << Hazel::RESET << std::endl;


/* ����һЩҪ�õ��ĳ��� */
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
    const int MAXACTIVELEN = 100;  // ��������󳤶�
    const int MAXLINELEN = 321;  // ÿһ�е���󳤶�
    const int MAXBLOCKLEN = 81;  // ÿһ���п����󳤶�
    const int MAXFILENAMELEN = 100;  // �ļ�������󳤶�
    // �ַ���ƥ���㷨��ѡ��
    const int KMP = 1;
    const int TRIE = 2;
    const int BF = 3;
};


/* �ṹ��Ķ��� */

// �洢ÿһ�е��ı�����ʽ�洢
typedef struct LineBlock {
    char data[Hazel::MAXBLOCKLEN];  // ����ÿһ����ַ�����
    LineBlock* next;  // ָ����һ���п�
    // Ĭ�Ϲ���
    LineBlock(const char* text, LineBlock* ne = nullptr)
    {
        int length = strlen(text);
        memcpy(data, text, std::min(length, Hazel::MAXLINELEN - 1));  // ��text�����ݿ�����data
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


/* ���������� */

/* ��ʾ���� */
void showMainMenu();
void showHelpMenu();
void showActiveArea(const ActiveArea&, int, int, int);
void printLine(const LineBlock*);
void printPositions(Pos[], int);
/* �������� */
void readFile(char*, char*);
int getValidInput();
void getNumber(const char*, int&, int, int);
void getString(const char*, char*, int, int);
bool CHECK_AREA(const ActiveArea&);
bool CHECK_BLOCK(const LineBlock*);
bool CHECK_BOUND(int, int, int, int low = 0);
/* ��Line������һЩ���� */
bool initArea(ActiveArea&);
int countLine(const ActiveArea&);
bool emptyArea(const ActiveArea&);
bool insertLine(ActiveArea&, char*, int, ActiveArea&, char* output_file = nullptr);
void changeActiveArea(ActiveArea&, ActiveArea&, char*);
bool deleteLine(ActiveArea&, ActiveArea&, int, int);
bool matchString(ActiveArea&, char*, int, int, Pos[], int&, int);
bool replaceString(ActiveArea&, int, char*, char*, int&, int type = Hazel::KMP);
bool replaceString(ActiveArea&, char*, char*, int, int, int&, int type = Hazel::KMP);
/* ��LineBlock������һЩ���� */
bool insertLineBlock(LineBlock*&, char*);
bool emptyLineBlock(const LineBlock*);
bool clearLineBlock(LineBlock*&);
void blocks_to_str(LineBlock*, char*);
void str_to_blocks(char*, LineBlock*&);
/* ���ļ�������һЩ���� */
bool readFromInputFile(char*, ActiveArea&, ActiveArea&, char* ouput_file = nullptr);
bool writeToOutputFile(char*, ActiveArea&, ActiveArea&, int, int);
bool readFromOtherArea(ActiveArea&, ActiveArea&, int);
/* ���ַ���������һЩ���� */
bool insertString(char*, int, const char*);
bool eraseString(char*, int, int);
bool kmp(char*, char*, int[], int&);
bool trie(char*, char*, int[], int&);
bool bf(char*, char*, int[], int&);


/* �����ľ���ʵ�� */


/* 1. ��ʾ������ʵ�� */

// ��ʾ����������
void showMainMenu()
{
    INFO("************  Welcome to Hazel Editor  ************")
    std::cout << Hazel::CYAN << "Please input your operation: (If you are new, you can type help to get help menu)" << Hazel::RESET << std::endl;
}
// ��ʾ�����˵�
void showHelpMenu()
{
    INFO("help menu")
    std::cout << Hazel::CYAN << "********************************************************************************" << Hazel::RESET << std::endl;
    std::cout << Hazel::CYAN << "*" << Hazel::RESET << " 1. Insert Line: Format - i<line_number><Enter><text><Enter>.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 2. Delete Line: Format - d<line_number1>[ <line_number2>].\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 3. Switch Active Area: Format - n\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 4. Display Active Area: Format - p\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 5. Replace String: Format - S<line_number1>( <line_number2>)@<string1>@<string2>.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 6.  Match String: Format - m <string>[ <line_number1>[ <line_number2>]].\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 7. Read From File: Format - r <file_name>.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 8. Write To File: Format - w <file_name>[ <line_number1>[ <line_number2>]].\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 9. Exit Editor: Format - exit" << std::endl;
    std::cout << Hazel::CYAN << "********************************************************************************" << Hazel::RESET << std::endl;
}
// ��ʾ�������ӵ�begin�п�ʼ��ʾ��һ����ʾcnt��
void showActiveArea(const ActiveArea& activeArea, int begin, int cnt, int page_num)
{
    INFO("---------------  Current Active Area (Page " << page_num << ")  ---------------")
    if (emptyArea(activeArea))
    {
        WARNINGS("    Current Active Area is None!\n")
        return;
    }
    // �����ǰ����
    Line* cur = activeArea;
    while (cur && begin -- ) cur = cur->next;  // �ҳ�Ҫ��ʾ�Ļ�����ʼ�е�begin��
    if (!cur)  // ��curΪ�գ�˵������������β��
    {
        WARNINGS("    Current Active Area is None!\n")
        return;
    }
    // ��ʱcurָ���begin��
    while (cur && cnt -- )
    {
        std::cout << Hazel::BLUE << std::setw(4) << cur->line_no << Hazel::RESET << ' ';  // ��ʾ�кţ���ɫ��ʾ��
        printLine(cur->content);
        cur = cur->next;
    }
    std::cout << Hazel::YELLOW << std::setw(60) << "total line: " << activeArea->line_no << Hazel::RESET << std::endl;  // ��ʾ����������ɫ��ʾ��
    std::cout << std::endl;
}
// ��ӡһ�е��ı�����
void printLine(const LineBlock* line_content)
{
    while (line_content)
    {
        std::cout << line_content->data;
        line_content = line_content->next;
    }
    std::cout << std::endl;
}
// ��ӡλ��(row,col)����
void printPositions(Pos positions[], int n)
{
    for (int i = 0; i < n; i ++ )
        std::cout << i + 1 << "-th match position: " << Hazel::CYAN <<positions[i].row << "," << positions[i].col << Hazel::RESET << std::endl;
}


/* 2. ����������ʵ�� */

// ��ȡ��������ļ���
void readFile(char* input_file, char* output_file)
{
    std::cout << "Please enter Input File: ";
    std::cin.getline(input_file, Hazel::MAXFILENAMELEN);
    std::cout << "Please enter Output File: ";
    std::cin.getline(output_file, Hazel::MAXFILENAMELEN);
}
// ��ȡ��Ч����
int getValidInput()
{
    int input;
    while (true)
    {
        std::cin >> input;
        if (std::cin.fail())  // ���������Ч��
        {
            WARNINGS("Invalid Input! Please input a number")
            std::cin.clear();  // �������״̬
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // ������Ч������
        }
        else break;
    }
    return input;
}
// ��ȡ��src�д��±�Ϊ[begin,end]�������ڵ����ָ�res ����end������Χ��ʾ�����ַ���������
void getNumber(const char* src, int& res, int begin, int end)
{
    if (!CHECK_BOUND(begin, end, strlen(src), 0)) return;
    res = 0;
    for (int i = begin; src[i] && i <= end; i ++ ) res = res * 10 + (src[i] - '0');
}
// ��ȡsrc���ִ�src[begin,end]��dist
void getString(const char* src, char* dist, int begin, int end)
{
    if (!CHECK_BOUND(begin, end, strlen(src), 0)) return;
    int cnt = 0;
    for (int i = begin; i <= end; i ++ ) dist[cnt ++ ] = src[i];
    dist[cnt] = '\0';
}
// ���ActiveArea�Ƿ�Ϸ�
bool CHECK_AREA(const ActiveArea& area)
{
    if (!area)
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


/* 3. ��Area��������غ�����ʵ�� */

// ��ʼ��ActiveArea
bool initArea(ActiveArea& area)
{
    try
    {
        area = new Line(0, nullptr, nullptr);
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
    if (!CHECK_AREA(area)) return -1;
    return area->line_no;
}
// �ж�ActiveArea�Ƿ�Ϊ��
bool emptyArea(const ActiveArea& area)
{
    if (!CHECK_AREA(area)) return false;
    return area->line_no == 0/* && area->next == nullptr*/;
}
// ���в��뵽��i�к��棬��output_fileΪ��ʱ˵���������ǻ����в�������
bool insertLine(ActiveArea& activeArea, char* text, int i, ActiveArea& otherArea, char* output_file)
{
    if (i < 0 || i > countLine(activeArea))  // ����λ�ò��Ϸ�
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
        initArea(activeArea);  // ��ʼ��activeArea����ͷ�ڵ㣩
        activeArea->next = new_line;
        activeArea->line_no = 1;  // ���µ�ǰ����Ϊ1
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
    if (activeArea->line_no > Hazel::MAXACTIVELEN && output_file)
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
    if (!CHECK_AREA(activeArea) || !CHECK_BOUND(begin, end, countLine(activeArea), 1)) return false;
    cnt = 0;  // cnt��ʼ����0
    int len = end - begin + 1;  // len�洢�ܹ��ж�������Ҫƥ��
    Line* cur = activeArea;
    while (cur->next && begin -- ) cur = cur->next;  // �ҵ���begin�У�����cntָ���begin��
    while (cur && len -- )
    {
        char line_content[Hazel::MAXLINELEN];  // �洢��ǰ�е��ı�����
        blocks_to_str(cur->content, line_content);  // ����ǰ�п�ת��һ������ַ�����
        // �ڵ�ǰ���ڽ���ƥ�䣨kmp��
        int pos[Hazel::MAXLINELEN] = {0}, matched_cnt = 0;  // pos��λ���±��0��ʼ������±���Ҫ+1
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
    INFO("Match Ends")
    return cnt > 0;  // ��cnt����0��˵��ƥ�䵽�ˣ�����û��ƥ�䵽
}
// �������е�line_no�е��ַ���original�滻���ַ���replaced��cnt��¼�滻�ɹ�������typeָ��ƥ���㷨��Ĭ��ʹ��KMP�㷨
bool replaceString(ActiveArea& activeArea, int line_no, char* original, char* replaced, int& cnt, int type)
{
    if (!CHECK_AREA(activeArea)) return false;
    if (line_no < 1 || line_no > countLine(activeArea))
    {
        ERROR("The parameter 'line_no' is invalid")
        return false;
    }
    int tmp = line_no;  // ��ʱ������¼Ŀ���к�
    char line_content[Hazel::MAXLINELEN];  // �洢��ǰ�������ı�����
    Line* cur = activeArea;
    while (cur->next && tmp -- ) cur = cur->next;  // �ҵ���line_no��
    blocks_to_str(cur->content, line_content);  // ����line_no�е�ÿһ��ϲ���һ���������ַ�����ֵ��line_content
    // �ַ���ƥ��
    int matched[Hazel::MAXLINELEN] = {0};  // matched�洢ƥ��ɹ����±�
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
        if (idx > Hazel::MAXLINELEN) break;  // ��������Χ��ֱ���˳����滻����
        eraseString(line_content, idx, idx + original_length - 1);  // ɾ���ַ���line_content�е�original�ַ���
        insertString(line_content, idx, replaced);  // �ڵ�idx���ַ���������ַ���replaced
        last += replaced_length - original_length;  // ����ƫ����last
    }
    str_to_blocks(line_content, cur->content);  // ���޸ĺ��line_content��������cur->content
    // !ע�⣺������ʱ����LineBlock* content��¼cur->content����Ѳ�����Ľ��ӳ�䵽activeArea��
    return cnt > 0;
}
// �������е�[begin,end]�����������е��ַ���original�滻���ַ���replaced��cnt��¼�滻�ɹ�������typeָ��ƥ���㷨��Ĭ��ʹ��KMP�㷨
bool replaceString(ActiveArea& activeArea, char* original, char* replaced, int begin, int end, int& cnt, int type)
{
    if (!CHECK_AREA(activeArea) || !CHECK_BOUND(begin, end, countLine(activeArea), 1)) return false;
    for (int i = begin; i <= end; i ++ )
    {
        int line_match_cnt = 0;  // �洢ÿһ�гɹ�ƥ��Ĵ���
        replaceString(activeArea, i, original, replaced, line_match_cnt, type);
        cnt += line_match_cnt;
    }
    INFO("Replace Ends")
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
    char block_content[Hazel::MAXBLOCKLEN];  // �洢ÿһ����ı�
    int idx = 0;  // ��¼src���±�����
    while (len > 0)  // ���δ���ÿһ��
    {
        int block_len = 0;  // ��¼��ǰ��Ĵ�С
        for (int i = idx; src[i]; i ++ )  // ��src�е����ݿ�����ÿһ����
        {
            if (block_len == Hazel::MAXBLOCKLEN - 1) break;
            block_content[block_len ++ ] = src[i];
        }
        block_content[block_len] = '\0';  // �����һ���ַ���Ϊ'\0'����ʾ�����
        insertLineBlock(dist, block_content);  // ���ÿ����ݲ��뵽dist��
        len -= Hazel::MAXBLOCKLEN - 1;  // ����len
    }
}


/* 5. ���ļ�������һЩ���� */

// �������ļ��е����ݶ�������У���ȡʧ�ܷ���false����output_fileΪ�գ����ʾ����ǻ�����
bool readFromInputFile(char* input_file, ActiveArea& activeArea, ActiveArea& otherArea, char* output_file)
{
    std::ifstream ifs(input_file);
    if (!ifs.is_open())
    {
        ERROR("Input File: '" << input_file << "' opening failed")
        return false;
    }
    std::cout << "Reading from the file: '" << Hazel::MAGENTA << input_file << Hazel::RESET <<  "' ...\n";
    char text[Hazel::MAXLINELEN];
    int cnt = countLine(activeArea);  // cnt��¼��ǰ��������
    // ˵�������������ж������ݣ���ʱ�ǻ�������Ϊ�գ�
    if (output_file != nullptr)
    {
        while (ifs.getline(text, Hazel::MAXLINELEN) && cnt < Hazel::MAXACTIVELEN - 20)  // ����ȡ��ACTIVEMAXLEN - 20��
            insertLine(activeArea, text, cnt ++ , otherArea, output_file);  // ��text���뵽��cnt�к���
        activeArea->line_no = cnt;  // ���»����ĳ��ȣ���insertLine����ʵ���Զ����µ�ǰ�������ȣ�
        // ��ʣ�ಿ�ֶ���ǻ������ǻ����޳�������
        if (strlen(text) != 0) insertLine(otherArea, text, 0, activeArea, nullptr);  // ��ȡ��81�е��ǻ�����
        cnt = 1;  // ��ʱcnt��¼�ǻ������ȣ��������ӵ�82�п�ʼ�������Ҵ�otherArea�ĵ�1�п�ʼд
        while (ifs.getline(text, Hazel::MAXLINELEN)) insertLine(otherArea, text, cnt ++ , activeArea, nullptr);  // ʣ�ಿ�ֶ���ǻ���
    }
    // ˵���������ǻ����ж�������
    else
    {
        while (ifs.getline(text, Hazel::MAXLINELEN)) insertLine(otherArea, text, cnt ++ , activeArea, nullptr);
    }
    INFO("End of Reading")
    ifs.close();  // �ر��ļ�������
    return true;
}
// ������������[begin, end]�е�����д������ļ�
bool writeToOutputFile(char* output_file, ActiveArea& activeArea, ActiveArea& otherArea, int begin, int end)
{
    std::ofstream ofs(output_file, std::ios::app);  // ��׷�ӵķ�ʽ���ļ�
    if (!ofs.is_open())
    {
        ERROR("Output file opening failed")
        return false;
    }
    if (!CHECK_BOUND(begin, end, countLine(activeArea))) return false;
    // �������ڵ�������д������ļ���
    std::cout << "Writing to the file: '" << Hazel::MAGENTA << output_file << Hazel::RESET <<"' ...\n";
    int len = end - begin + 1;  // len�洢Ҫд������ļ���������
    char text[Hazel::MAXBLOCKLEN];
    Line* cur = activeArea;
    while (cur->next && -- begin) cur = cur->next;  // �ҵ���begin�е�ǰ���ڵ�
    Line* left = cur;  // leftָ���begin�е�ǰ��
    cur = cur->next;  // ��curָ���begin��
    for (int i = 0; i < len && cur; i ++ )  // ����д���begin�е���end��
    {
        Line* tmp = cur;
        char text[Hazel::MAXLINELEN];
        blocks_to_str(cur->content, text);  // ���п�ת��char�ַ�����
        ofs << text << std::endl;  // ����д������ļ���
        cur = cur->next;  // ������һ��
        clearLineBlock(tmp->content);  // ��յ�ǰ��
        delete tmp;
    }
    left->next = cur;  // ����left�к�end��һ����������
    // ������left�к��������е��к�
    while (cur)
    {
        cur->line_no -= end;  // ������кż�ȥend����
        cur = cur->next;
    }
    activeArea->line_no -= len;  // ���»�����ǰ������
    // ���ǻ����л������ݣ���ӷǻ����ж���len�е�������
    if (!emptyArea(otherArea))
    {
        readFromOtherArea(activeArea, otherArea, len);  // ��Ҫ��ȡlen�зǻ���������
    }
    INFO("End of Writing")
    ofs.close();  // �ر�����ļ���
    return true;
}
// ���ǻ����е�len��д������� ��������len�У���ȫ��д�������
bool readFromOtherArea(ActiveArea& activeArea, ActiveArea& otherArea, int len)
{
    if (!CHECK_AREA(activeArea) || !CHECK_AREA(otherArea)) return false;
    int actualLength = 0;  // �洢ʵ��д�������
    Line* tail = activeArea;  // tailָ�������β�ڵ�
    while (tail->next) tail = tail->next;
    // ��otherArea��ǰlen�ж���activeArea
    Line* head = otherArea->next;  // headָ��ǻ�������Ԫ�ڵ�
    while (head && len -- )
    {
        tail->next = head;
        otherArea->next = head->next;  // otherArea����Ԫ�ڵ����
        head = otherArea->next;  // ����headʼ��ָ��ǻ�������Ԫ�ڵ�
        tail->next->line_no += 10;  // �ǻ�������������ÿһ�е�line_no��Ҫ����ƫ����(10)
        tail = tail->next;  // tail����
        tail->next = nullptr;  // tail��nextָ���ÿ�
        actualLength ++ ;  // ʵ��д������+1
    }
    otherArea->next = head;  // ����otherArea��nextָ��ָ��ǻ�������Ԫ�ڵ�head
    // ���»����ͷǻ����ĳ���
    otherArea->line_no -= actualLength;
    activeArea->line_no += actualLength;
    INFO("Read from Other Area successfully")
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
        if (i >= Hazel::MAXLINELEN - 1) continue;  // ���������Χ������
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
    if (n > m) return false;
    char p[Hazel::MAXLINELEN + 1], s[Hazel::MAXLINELEN + 1];  // p��ģʽ����s��ĸ����ƫ����+1�����ü���
    memcpy(p + 1, match_str, std::min(n, Hazel::MAXLINELEN - 1));
    memcpy(s + 1, str, std::min(m, Hazel::MAXLINELEN - 1));
    p[n + 1] = s[m + 1] = '\0';  // ��ǽ�����
    // ��next����
    int ne[Hazel::MAXLINELEN + 1] = {0};
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
    if (strlen(match_str) > strlen(src)) return false;
    return tr.search(src, match_str, pos, cnt);
}
// bf��O(nm)��ƥ��ĸ��str�е�����ģʽ��match_str��������λ�÷���pos�����У�����Ϊcnt����ƥ��ɹ�������true
bool bf(char* str, char* match_str, int pos[], int& cnt)
{
    cnt = 0;
    int n = strlen(str), m = strlen(match_str);
    if (m > n) return false;
    // ����ö��ÿ��ƥ������
    for (int i = 0; i <= n - m; i ++ )
    {
        int j;
        for (j = 0; j < m; j ++ )  // ö��ƥ�䳤��
            if (str[i + j] != match_str[j])
                break;
        // ��ƥ��ɹ�������λ��
        if (j == m) pos[cnt ++ ] = i;
    }
    return cnt > 0;
}


/* ������ */
int main()
{
    ActiveArea activeArea = nullptr, otherArea = nullptr;
    initArea(activeArea);
    initArea(otherArea);

    char input_file[Hazel::MAXFILENAMELEN + 1] = "../data/input/input.txt", output_file[Hazel::MAXFILENAMELEN + 1] = "../data/output/output.txt";
    readFile(input_file, output_file);
    
    // ���������ļ��������ó���������ļ��ж�ȡ�ı�
    if (input_file)
    {
        readFromInputFile(input_file, activeArea, otherArea, output_file);
    }
    // ��ʼ�������
    char op[Hazel::MAXLINELEN] = {0};
    int count = 0;  // count��¼��ǰ�Ѿ����ж��ٴβ���
    while (true)
    {
        // ��ӡ�ϴβ���
        TRACE("last operation: " + (strlen(op) == 0 ? std::string("none") : std::string(op)))
        // ��ʾ����
        showActiveArea(activeArea, 1, countLine(activeArea), 1);
        // չʾ����������
        showMainMenu();
        // ���뱾�β���
        std::cin.getline(op, sizeof op);
        // ��������+1
        count ++ ;

        /* �жϲ������� */
        // 1. �˳�ϵͳ
        if (strcmp(op, "exit") == 0)
        {
            break;
        }
        // 2. ��ʾ�����˵�
        else if (strcmp(op, "help") == 0)
        {
            showHelpMenu();
            continue;
        }
        // 3. �в���
        else if (op[0] == 'i')
        {
            char str[Hazel::MAXLINELEN];
            std::cin.getline(str, sizeof str);
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
                    TRACE("End of Active Area")
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // �������һ��'Y'�Ļس�
                    break;
                }
                std::cout << "Do you want to continue to the next page? (Y/N): ";
                char choice; std::cin >> choice;  // ����ѡ��
                if (choice != 'Y') break;  // ����������ʾ������ʾ��ϣ��˳�
            }
        }
        // 7. �ַ����滻
        else if (op[0] == 'S')
        {
            int begin = 0, end = 0;
            int i, j;
            for (i = 1; op[i] != '@'; i ++ );  // ��iָ���һ��'@'��
            for (j = i + 1; op[j] != '@'; j ++ );  // ��jָ��ڶ���'@'��
            char original[Hazel::MAXLINELEN], replaced[Hazel::MAXLINELEN];
            int k = 1;
            while (op[k] && op[k] != ',') k ++ ;  // ��kָ�򶺺Ŵ������ַ���������'\0'����
            if (op[k] == ',')  // ˵������<�к�2>
            {
                getNumber(op, begin, 1, k - 1);  // ��ȡ<�к�1>
                getNumber(op, end, k + 1, i - 1);  // ��ȡ<�к�2>
            }
            else
            {
                getNumber(op, begin, 1, i - 1);  // ����ȡ<�к�1>
                end = begin;
            }
            getString(op, original, i + 1, j - 1);  // ��ȡԭ��
            getString(op, replaced, j + 1, strlen(op) - 1); // ��ȡ�滻��
            int replaced_cnt = 0;  // �洢�滻����
            replaceString(activeArea, original, replaced, begin, end, replaced_cnt);  // �����滻�ַ���������Ĭ��ʹ��KMP��
            std::cout << Hazel::CYAN << "total replaced counts: " << replaced_cnt << std::endl;
        }
        // 8. �ַ���ƥ��
        else if (op[0] == 'm')
        {
            char match_str[Hazel::MAXLINELEN];
            int i = 1;
            while (op[i] && op[i] == ' ') i ++ ;  // ���˿ո���iָ���ƥ�䴮����ʼ�±�
            int j = i;
            while (op[j] && op[j] != ' ') j ++ ;  // ��jָ����һ���ո񴦣����ַ���������'\0'����
            getString(op, match_str, i, j - 1);  // ��ȡ��ƥ���ַ���
            i = j;
            while (op[i] && op[i] == ' ') i ++ ;  // ���˿ո���iָ��<�к�1>��ʼ�±괦
            j = i;
            while (op[j] && op[j] != ' ') j ++ ;  // ����<�к�1>����jָ����һ���ո񴦣���'\0'����
            int begin = 0, end = 0;
            if (i == j)  // ˵��ֻ�����ƥ�䴮
            {
                begin = 1, end = countLine(activeArea);
            }
            else
            {
                getNumber(op, begin, i, j - 1);  // ��ȡ�к�1
                i = j;
                while (op[i] && op[i] == ' ') i ++ ;  // ���˿ո���iָ��<�к�2>��ʼ�±괦
                j = i;
                while (op[j] && op[j] != ' ') j ++ ;  // ����<�к�2>����jָ�����ս�����'\0'��
                if (i == j)  // ˵��ֻ������<�к�1>
                    end = begin;
                else  // ���򣬻�ȡ<�к�2>
                    getNumber(op, end, i, j - 1);
            }
            Pos positions[Hazel::MAXLINELEN * Hazel::MAXACTIVELEN] = {0};  // �洢ƥ��ɹ���λ��
            int matched_cnt = 0;  // ��¼ƥ��ɹ��ĸ���
            // ƥ�����������match
            bool is_matched = matchString(activeArea, match_str, begin, end, positions, matched_cnt);
            std::cout << Hazel::CYAN << "total matches: " << matched_cnt << Hazel::RESET << std::endl;
            if (is_matched) printPositions(positions, matched_cnt);  // ��ƥ��ɹ������ӡ����ƥ�䵽��λ��
        }
        // 9. ��ȡ�����ļ��������У�ǰ���Ƿǻ����������ݣ�
        else if (op[0] == 'r')
        {
            int i = 1;
            while (op[i] && op[i] == ' ') i ++ ;  // ���˿ո�iָ��<�ļ���>��ʼ�±�
            getString(op, input_file, i, strlen(op) - 1);
            if (!emptyArea(otherArea))  // ���ǻ����л��������ݣ����޷���ȡ
            {
                WARNINGS("Other Area is not empty, can't read from Input File: '" <<  input_file << "\'")
                continue;
            }
            readFromInputFile(input_file, activeArea, otherArea, output_file);
        }
        // 10. д������ļ�
        else if (op[0] == 'w')
        {
            int i = 1;
            while (op[i] && op[i] == ' ') i ++ ;  // ���˿ո�iָ��<�ļ���>��ʼ�±�
            int j = i;
            while (op[j] && op[j] != ' ') j ++ ;  // �����ļ�����jָ����һ���ո񴦣���'\0'����
            getString(op, output_file, i, j - 1);  // ��ȡ<�ļ���>
            while (op[j] && op[j] == ' ') j ++ ;  // ���˿ո�jָ��<�к�1>��
            i = j;
            while (op[i] && op[i] != ' ') i ++ ;  // ���˵�һ�����֣�iָ����һ���ո񴦣���'\0'����
            int begin = 0, end = 0;
            if (i == j) begin = 1, end = countLine(activeArea);  // ˵��ֻ������<�ļ���>
            else
            {
                getNumber(op, begin, j, i - 1);  // ��ȡ<�к�1>
                while (op[i] && op[i] == ' ') i ++ ;  // ���˿ո�iָ��<�к�2>��
                j = i;
                while (op[j]) j ++ ;  // ���˵ڶ������֣�jָ������ַ���������'\0'��
                if (i == j) end = begin;  // ˵��ֻ������<�к�1>
                else getNumber(op, end, i, j - 1);  // ��ȡ<�к�2>
            }
            // д���ȡ������[begin,end]�е����ݵ�����ļ���
            writeToOutputFile(output_file, activeArea, otherArea, begin, end);
        }
        // 9. �Ƿ�����
        else
        {
            if (count == 1)  // ��һ�β��ǷǷ�����
            {
                clearScreen();
                continue;
            }
            WARNINGS("Invalid Input!")
        }
        // ���»س��Լ���
        std::cout << Hazel::GREEN << "Press enter to continue..." << Hazel::RESET;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // �ж�ֻ���ջس�
        clearScreen();  // ÿ����һ�κ�����
    }

    return 0;
}