#include "exp_env.h"
#include "exp_setting.h"
#include "rpq/rpq_exp.h"

using namespace std;

int main(int argc, char *argv[])
{
    // time_max();
    // test_count_dis_one(ips[0],6379);

    istream::sync_with_stdio(false);
    ostream::sync_with_stdio(false);

    if (argc == 2)
        exp_env::sudo_pwd = argv[1];
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
    //list_exp le;

    exp_setting::compare = false;
    re.test_default_settings();
    //re.test_patterns();
    // le.test_default_settings();
    // le.test_patterns();
    // exp_setting::compare = true;
    // re.test_default_settings();
    // re.test_patterns();
    // le.test_default_settings();
    // le.test_patterns();

    //exp_setting::compare = false;
    // for (int i = 0; i < 16; i++)
    // {
    //     re.test_delay(i);
    //     re.test_replica(i);
    //     re.test_speed(i);
    // }

    //re.test_delay(1);

    // exp_setting::compare = true;
    // for (int i = 0; i < 16; i++)
    // {
    //     le.test_delay(i);
    //     le.test_replica(i);
    //     le.test_speed(i);
    // }

    return 0;
}
