# TIL compiler

IST Compilers course project, 2023/2024. Implements a compiler for the language TIL, specified [here](https://web.tecnico.ulisboa.pt/~david.matos/w/pt/index.php/Compiladores/Projecto_de_Compiladores/Projecto_2023-2024/Manual_de_Refer%C3%AAncia_da_Linguagem_TIL#Invoca.C3.A7.C3.A3o) (in portuguese).

It consisted of adapting the following from a very basic language that utilized the provided base libraries:
* the scanner (`til_scanner.l`)
* the parser (`til_parser.y`)
* the symbol (`targets/symbol.h`)
* the type checker (`targets/type_checker.cpp`)
* the XML writer (`targets/xml_writer.cpp`)
* the Postfix writer (`targets/postfix_writer.cpp`)

Along with creating all the new required nodes under `ast/`. 

---

**This project uses the libraries CDK19 (Compiler Development Kit) and RTS5 (Run Time System) as a base**. Both are available for download under "Material de Uso Obrigatório" (EN: materials of obligatory usage) in the specification page.
