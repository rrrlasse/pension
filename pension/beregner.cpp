#include <iostream>

int main()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Beregn, hvad du får af det offentlige, når du går på pension. 100% utestet ind til videre
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool samlevende = true;             // Har du samlever?
    bool elvarme = false;               // Har du elvarme?
    bool gasvarme = false;              // Har du gasvarme?
    double yderligere_over_18 = 0;         // Yderligere personer over 18 på bopælen udover samlever

                                        // Angiv dine ikke-offnetlige indtægter per år for dig + eventuel samlever og før skat:
    double private_pensioner = 20000;      // ATP, ratepensioner, arbejdsmarkedspensioner
    double kapitalindkomst = 10000;            // Positive nettoindkomst af renter, aktier, mv
    double aktieindkomst = 10000;           // Aktieandel af ovenstaaende
    double personlig_indkomst = 100050;    // Løn + honorar + selvstændig virksomhed

                                        // Angiv dine udgifter per år for dig + eventuel samlever:
    double varmeudgift = 10000;            // Boligens udgifter til opvarmning
    double ejendomsvaerdiskat = 20000;     // Ejendomsværdiskat af evt. hus
    double aarlig_medielicens = 2460;      // 2017 sats

                                        // Diverse
    double aldersopsparing = 100000;       // Størrelsen af denne engangsudbetaling, hvis du har tilvalgt denne
    double formue = 0;                     // Din eventuelle formue *før* udbetaling af aldersopsparingen

                                        // TODO: Boligydelse!
                                        // TODO: Nye modregningsfrie pensionsopsparingsordning, når den er færdig...

                                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    formue += aldersopsparing;
    double personlig_tillaegsprocent;

    double aarlig_total_beskattet = 0;
    double aarlig_total_ubeskattet = 0;
    double aarlig_fradrag_total = 0;

    // Først beregner vi en konstant kaldet "personlig tillægsprocent", som anvendes i beregningerne af mange
    // fradrag, tilskud, mv.
    {
        double relevant_indkomst = kapitalindkomst + personlig_indkomst + private_pensioner;
        if (!samlevende && relevant_indkomst <= 20100 ||
            samlevende && personlig_indkomst <= 39800) {
            personlig_tillaegsprocent = 1.;
        }
        else if (!samlevende && relevant_indkomst >= 69800 ||
            samlevende && relevant_indkomst >= 140000) {
            personlig_tillaegsprocent = 0;
        }
        else {
            personlig_tillaegsprocent = 1.;
            double over = relevant_indkomst - (!samlevende ? 19700 : 38800);
            if (over > 0) {
                // TODO: Lovteksten antyder, at det muligvis kunne være nedrundet heltalsdivision
                personlig_tillaegsprocent -= over / (samlevende ? 1002. : 496.) / 100.;
            }
        }
    }

    // Folkepensionens grundbeløb.
    {
        double grundbeloeb = 73920;
        double relevant_indkomst = personlig_indkomst;

        // Rund ned til nærmeste 100
        relevant_indkomst = relevant_indkomst / 100 * 100;

        if (relevant_indkomst > 316200) {
            double overskrid = relevant_indkomst - 316200;
            // TODO: Lovteksten antyder, at det muligvis kunne være nedrundet heltalsdivision
            grundbeloeb -= (overskrid / 100.) * 30.00;

            if (grundbeloeb < 0)
                grundbeloeb = 0;
        }

        aarlig_total_beskattet += grundbeloeb;
        std::cout << "Folkepensionens grundbeløb: " << grundbeloeb << "\n";

    }

    // Folkepensionens pensionstillæg.
    {
        double tillaeg = 78612;
        double relevant_indkomst = kapitalindkomst + personlig_indkomst + private_pensioner;

        // Rund ned til nærmeste 100
        relevant_indkomst = relevant_indkomst / 100 * 100;

        if (relevant_indkomst > 69800) {
            double overskrid = relevant_indkomst - 69800;
            tillaeg -= (overskrid / 100.) * 30.90;

            if (tillaeg < 0)
                tillaeg = 0;
        }

        aarlig_total_beskattet += tillaeg;
        std::cout << "Folkepensionens pensionstillæg: " << tillaeg << "\n";

    }

    // Mulighed for reduceret medielicens
    {
        double reduktion;
        if (personlig_tillaegsprocent == 1.) {
            reduktion = aarlig_medielicens / 2;
        }
        else {
            reduktion = 0;
        }
        std::cout << "Tilskud til medielicens: " << reduktion << "\n";
        aarlig_fradrag_total += reduktion;
    }

    // Ældrecheck
    {
        double check = 16900;
        if (formue < 84300 || personlig_tillaegsprocent * check < 200) {
            // Ingen check hvis du har for stor formue, eller checken ender under 200 kr.
            check = 0;
        }
        else {
            check = check * personlig_tillaegsprocent;
        }
        aarlig_total_beskattet += check;
        std::cout << "Ældrecheck: " << check << "\n";
    }

    // Pensionistnedslag i ejendomsværdiskat.
    {
        double nedslag;
        // Ubeskattet. Formue ingen betydning.
        if (ejendomsvaerdiskat > 0) {
            nedslag = 0.04 * ejendomsvaerdiskat;

            double saerligt_indkomstgrundlag = personlig_indkomst +
                kapitalindkomst +
                (aktieindkomst > 10000 ? aktieindkomst - 10000 : 0);

            double overskrid = saerligt_indkomstgrundlag > 281300 ? saerligt_indkomstgrundlag - 281300 : 0;
            double nedslag_af_nedslag = 0.05 * overskrid;
            nedslag -= nedslag_af_nedslag;

            // Loft på nedslag er 6000 DKK / år
            if (nedslag > 6000)
                nedslag = 6000;
        }
        else {
            nedslag = 0;
        }
        aarlig_fradrag_total += nedslag;
        std::cout << "Nedslag i ejendomsværdiskat: " << nedslag << "\n";
    }

    // Varmetillæg.
    {
        double tillaeg;
        if (varmeudgift > 0) {
            tillaeg = 0;
            double optil1 = samlevende ? 5000 : 7500;
            double optil2 = 12900;
            double optil3 = 17100;
            double optil4 = 21200;

            // Kun optil4 øges ved flere personer i husstanden
            if (yderligere_over_18 > 0)
                optil4 += 6400;

            double grundlag = varmeudgift > optil4 ? optil4 : varmeudgift;

            // Fuld egenbetaling mellem 0 og optil1
            if (grundlag > optil3)
                tillaeg += 0.25 * (grundlag - optil3);

            // 50% tilskud mellem optil1 og optil2
            if (grundlag > optil2)
                tillaeg += 0.5 * ((grundlag > optil3 ? optil3 : grundlag) - optil2);

            // 75% tilskud mellem optil2 til optil3
            if (grundlag > optil1)
                tillaeg += 0.75 * ((grundlag > optil2 ? optil2 : grundlag) - optil1);

            // Nesat tillaeg for elvarme
            if (elvarme && tillaeg > (samlevende ? 4400 : 3400))
                tillaeg -= (samlevende ? 4400 : 3400);

            // Nesat tillaeg for gasvarme
            if (gasvarme && tillaeg > (samlevende ? 900 : 800))
                tillaeg -= (samlevende ? 900 : 800);

            // Her bruges den magiske konstant beregnet i starten af programmet
            tillaeg = tillaeg * personlig_tillaegsprocent;

        }
        else {
            tillaeg = 0;
        }
        // Varmetillæg er ubeskattet
        aarlig_total_ubeskattet += tillaeg;
        std::cout << "Varmetillæg: " << tillaeg << "\n";
    }


    // Boligydelse
    {
        // TODO: Boligydelse kan ikke beregnes på en turingmaskine; er for komplekst.

        // Ældrecheck indgår *ikke* i opgørelsen af husstandens indkomst. ATP, folkepension og andre 
        // skattepligtige indkomster *indgår*

        // Formue *indgår*. "Formuetillægget udgør 10 % af formue over 826.500 kr.og 20 % af formue over 1.653.200 kr.
        // ved boligydelse.Formuetillægget udgør 10 % af formue over 753.800 kr.og 20 % af formue over 1.507.800 kr. 
        // ved boligsikring."

        // Formuetillæget adderes til din *årsindkomst* når boligydelsen beregnes.
    }

    std::cout << "\n\nTotale udbetalinger fra det offentlige:\n";
    std::cout << "Skattefrie:     DKK " << aarlig_fradrag_total + aarlig_total_ubeskattet << " / år\n";
    std::cout << "Skattepligtige: DKK " << aarlig_total_beskattet << " / år";

    // varme: https://www.borger.dk/pension-og-efterloen/Tillaeg-til-folke--og-foertidspension/folkepension-varmetillaeg
    // varme: https://www.aeldresagen.dk/viden-og-raadgivning/penge-og-pension/tillaeg-og-tilskud/gode-raad/varmetillaeg
    // licens: https://www.retsinformation.dk/forms/R0710.aspx?id=166986
    // ejendomsskat: http://www.skm.dk/skattetal/beregning/skatteberegning/ejendomsvaerdiskat-regler-og-beregningseksempler
    // boligstøtte: https://www.aeldresagen.dk/viden-og-raadgivning/vaerd-at-vide/b/bolig/boligstoette/hvad-indgaar-i-beregning-af-boligstoette
    // boligstøtte https://www.borger.dk/bolig-og-flytning/Boligstoette-oversigt/Boligstoette-soege/Boligstoette-info-om-formue
}

