
/*
writeIndels.cpp - This file is part of BayesTyper (https://github.com/bioinformatics-centre/BayesTyper)


The MIT License (MIT)

Copyright (c) 2016 Jonas Andreas Sibbesen and Lasse Maretty

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#include <unordered_map>
#include <unordered_set>
#include <assert.h>

#include "VcfFile.hpp"
#include "Variant.hpp"
#include "FastaRecord.hpp"
#include "FastaReader.hpp"
#include "Auxiliaries.hpp"


int main(int argc, char const *argv[]) {

    if (argc != 4) {

        std::cout << "USAGE: writeIndels <variant_file> <output_prefix> <min_indel_length>" << std::endl;
        return 1;
    }

    cout << "\n[" << Utils::getLocalTime() << "] Running BayesTyperTools (" << BT_VERSION << ") writeIndels script ...\n" << endl;


    VcfFileReader in_vcf(argv[1], true);
    
    ofstream indels_outfile(string(argv[2]) + ".fa");

    if (!indels_outfile.is_open()) {

        cerr << "\nERROR: Unable to write file " << string(argv[2]) + ".fa" << "\n" << endl;
        exit(1);
    }

    const uint min_indel_length = stoi(argv[3]); 

    Variant * cur_var;

    uint num_variants = 0;
    uint num_allelles = 0;
    uint num_allelles_written = 0;

    while (in_vcf.getNextVariant(&cur_var)) {

        num_variants++;

        for (uint alt_idx = 0; alt_idx < cur_var->numAlts(); alt_idx++) {

            num_allelles++;

            Allele ref = cur_var->ref();
            Allele alt = cur_var->alt(alt_idx);

            auto allele_attributes = Auxiliaries::alleleAttributes(alt, ref);
            Auxiliaries::fullTrimAllelePair(&ref, &alt);

            string out_seq;

            if (allele_attributes.type == Auxiliaries::Type::Insertion) {

                assert(ref.seq().size() == 0);
                assert(allele_attributes.sv_length > 0);

                out_seq = alt.seq();

            } else if (allele_attributes.type == Auxiliaries::Type::Deletion) {

                assert(alt.seq().size() == 0);
                assert(allele_attributes.sv_length < 0);

                out_seq = ref.seq();
            }

            if (out_seq.size() >= min_indel_length) {

                num_allelles_written++;

                string alt_id = cur_var->chrom() + "_" + to_string(cur_var->pos()) + "_" + to_string(alt_idx);
                FastaRecord alt_rec(alt_id, out_seq);
                indels_outfile << alt_rec.str();
            }

            if (num_variants % 100000 == 0) {

                cout << "[" << Utils::getLocalTime() << "] Parsed " << num_variants << " variant(s) ..." << endl;
            }
        }

        delete cur_var;
    }

    indels_outfile.close();

    cout << "[" << Utils::getLocalTime() << "] Completed BayesTyperTools writeIndelFasta\n" << endl;
    cout << "[" << Utils::getLocalTime() << "] " << num_variants << " variant(s) were parsed in total\n" << endl;
    cout << "[" << Utils::getLocalTime() << "] " << num_allelles << " allele(s) were parsed in total\n" << endl;
    cout << "[" << Utils::getLocalTime() << "] " << num_allelles_written << " allele(s) were written to fasta\n" << endl;  
}
