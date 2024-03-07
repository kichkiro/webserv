/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kichkiro <kichkiro@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/09 15:59:33 by kichkiro          #+#    #+#             */
/*   Updated: 2024/01/30 17:02:37 by kichkiro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFile.hpp"

int main(int argc, char const **argv) {
    try {
        if (argc == 1) {
            ConfigFile configfile;

            // X Gio ---------------------------------------------------------->

            // Ottieni il valore della prima direttiva inline contenuta nel primo blocco server
            cout << configfile.get_config()[0]->get_value_block()[0]->get_value_block()[0]->get_value_inline()[0] << endl;

            // Ottieni il tipo
            cout << configfile.get_config()[0]->get_value_block()[0]->get_value_block()[0]->get_type() << endl;

            // E' un blocco/contesto? (bool)
            cout << configfile.get_config()[0]->get_value_block()[0]->get_value_block()[0]->get_is_context() << endl;
        }
        else if (argc == 2)
            ConfigFile configfile(argv[1]);
        else {
            cerr << "Usage: ./webserv [CONFIG-FILE]" << endl;
            return 1;
        }
    }

    catch (const exception &e) {}
    
    return 0;
}

