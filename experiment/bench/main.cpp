#include "exp_env.h"
#include "exp_setting.h"
#include "rpq/rpq_exp.h"
// #include "list/list_exp.h"

using namespace std;

int main(int argc, char *argv[])
{
    istream::sync_with_stdio(false);
    ostream::sync_with_stdio(false);
    int round = 10;
    if (argc == 3)
    {
        if (strcmp(argv[1], "stop") == 0) {
            exp_env::sudo_pwd = argv[2];
            exp_env e(3, 1, 1, 1);
            return 0;
        } else {
            exp_env::sudo_pwd = argv[2];
            round = atoi(argv[1]);
        }
    }
    else if (argc == 1)
    {
        cout << "please enter the password for sudo: ";
        cin >> exp_env::sudo_pwd;
        cout << "\n";
    }
    else
    {
        cout << "error. too many input arguments." << endl;
        return -1;
    }

    rpq_exp re;
    // list_exp le;

    exp_setting::compare = false;
    re.test_default_settings(round);
    // le.test_default_settings();

    return 0;
}
