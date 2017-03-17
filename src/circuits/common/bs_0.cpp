/**
 \file 		millionaire_prob.cpp
 \author 	sreeram.sadasivam@cased.de
 \copyright	ABY - A Framework for Efficient Mixed-protocol Secure Two-party Computation
			Copyright (C) 2015 Engineering Cryptographic Protocols Group, TU Darmstadt
			This program is free software: you can redistribute it and/or modify
			it under the terms of the GNU Affero General Public License as published
			by the Free Software Foundation, either version 3 of the License, or
			(at your option) any later version.
			This program is distributed in the hope that it will be useful,
			but WITHOUT ANY WARRANTY; without even the implied warranty of
			MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
			GNU Affero General Public License for more details.
			You should have received a copy of the GNU Affero General Public License
			along with this program. If not, see <http://www.gnu.org/licenses/>.
 \brief		Implementation of the millionaire problem using ABY Framework.
 */

#include <bitset>
#include "bs_0.h"

int32_t test_millionaire_prob_circuit(e_role role, char* address, seclvl seclvl,
		uint32_t nvals, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg,
		e_sharing sharing) {

	/**
		Step 1: Create the ABYParty object which defines the basis of all the
		 	 	operations which are happening.	Operations performed are on the
		 	 	basis of the role played by this object.
	*/
	ABYParty* party = new ABYParty(role, address, seclvl, bitlen, nthreads,
			mt_alg);


	/**
		Step 2: Get to know all the sharing types available in the program.
	*/

	vector<Sharing*>& sharings = party->GetSharings();

	/**
		Step 3: Create the circuit object on the basis of the sharing type
				being inputed.
	*/
	Circuit* circ = sharings[sharing]->GetCircuitBuildRoutine();


	/**
		Step 4: Creating the share objects - s_alice_money, s_bob_money which
				is used as input to the computation function. Also s_out
				which stores the output.
	*/

    share *s_board;
    share *s_out;

	/**
		Step 5: Initialize Alice's and Bob's money with random values.
				Both parties use the same seed, to be able to verify the
				result. In a real example each party would only supply
				one input value.
	*/

    std::string bitstring = "10100"
                            "00001"
                            "01000"
                            "00011"
                            "10000";
    std::bitset<25> board(bitstring);
    uint32_t output = 0;

	/**
		Step 6: Copy the randomly generated money into the respective
				share objects using the circuit object method PutINGate()
				for my inputs and PutDummyINGate() for the other parties input.
				Also mention who is sharing the object.
	*/
	if(role == SERVER) {
        s_board = circ->PutINGate(board.to_ulong(), 25, SERVER);
	} else { //role == CLIENT
        s_board = circ->PutDummyINGate(25);
	}

	/**
		Step 7: Call the build method for building the circuit for the
				problem by passing the shared objects and circuit object.
				Don't forget to type cast the circuit object to type of share
	*/

	s_out = build_bs0_circuit(s_board, (BooleanCircuit*) circ);

	/**
		Step 8: Modify the output receiver based on the role played by
				the server and the client. This step writes the output to the
				shared output object based on the role.
	*/
	s_out = circ->PutOUTGate(s_out, ALL);

	/**
		Step 9: Executing the circuit using the ABYParty object evaluate the
				problem.
	*/
	party->ExecCircuit();

	/**
		Step 10:Type casting the value to 32 bit unsigned integer for output.
	*/
	output = s_out->get_clear_value<uint32_t>();

	cout << "Testing Millionaire's Problem in " << get_sharing_name(sharing)
				<< " sharing: " << endl;
    cout << bitstring << "\n";
	cout << "\nCircuit Result:\t" << (output ? true : false)
         << "\n";

	delete party;
	return 0;
}


share* build_bs0_circuit(share *s_board, BooleanCircuit *bc)
{
    share* out;
    std::vector<share*> cell_shares;
    share *s_one = bc->PutCONSGate(1, 1);;
    for (size_t row = 0; row < 5; ++row)
    {
        for (size_t col = 0; col < 5; ++col)
        {
            share *s_cell, *s_west, *s_north_west, *s_north, *s_north_east;
            share *s_tmp;

            s_cell = s_board->get_wire_ids_as_share(row * 5 + col);

            s_tmp = s_one;
            if (col > 0)
            {
                s_west = s_board->get_wire_ids_as_share(row * 5 + (col - 1));
                s_west = bc->PutINVGate(s_west);
                s_tmp = bc->PutANDGate(s_tmp, s_west);
            }
            if (row > 0 && col > 0)
            {
                s_north_west = s_board->get_wire_ids_as_share((row - 1) * 5 + (col - 1));
                s_north_west = bc->PutINVGate(s_north_west);
                s_tmp = bc->PutANDGate(s_tmp, s_north_west);
            }
            if (row > 0)
            {
                s_north = s_board->get_wire_ids_as_share((row - 1) * 5 + col);
                s_north = bc->PutINVGate(s_north);
                s_tmp = bc->PutANDGate(s_tmp, s_north);
            }
            if (row > 0 && col < 4)
            {
                s_north_east = s_board->get_wire_ids_as_share((row - 1) * 5 + (col + 1));
                s_north_east = bc->PutINVGate(s_north_east);
                s_tmp = bc->PutANDGate(s_tmp, s_north_east);
            }

            s_tmp = bc->PutINVGate(s_tmp);
            s_tmp = bc->PutANDGate(s_cell, s_tmp);
            s_tmp = bc->PutINVGate(s_tmp);
            cell_shares.push_back(s_tmp);
        }
    }
    assert(cell_shares.size() == 25);
    out = s_one;
    for (auto s : cell_shares)
    {
        out = bc->PutANDGate(out, s);
    }
    return out;
}
