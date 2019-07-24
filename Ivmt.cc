#include "Ivmt.hpp"

int main()
{
    Rebot rb;
    string input_msg, output_msg;
    while(true)
    {
        cout << "Mine: ";
        std::cin >> input_msg;
        output_msg = rb.Talk(input_msg);
        cout << "TuLing: " << output_msg << endl;
    }
    return 0;
}