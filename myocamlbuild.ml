open Ocamlbuild_plugin ;;

let _ = dispatch begin function
    | After_rules ->
        ocaml_lib "ozmq";

        flag ["ocamlmklib"; "c"; "use_zeromq"] (S[A"-lzmq"]);
        flag ["link"; "ocaml"; "use_zeromq"] (S[A"-cclib"; A"-lzmq"]);

        flag ["link";"library";"ocaml";"byte";"use_libozmqw"] & S[A"-dllib";A"-lozmqw";A"-cclib";A"-L.";A"-cclib";A"-lozmqw"];
        flag ["link";"library";"ocaml";"native";"use_libozmqw"] & S[A"-cclib";A"-L.";A"-cclib";A"-lozmqw"];
        flag ["link";"ocaml";"link_libozmqw"] (A"libozmqw.a");

        dep ["link";"ocaml";"use_libozmqw"] ["libozmqw.a"];
        dep ["link";"ocaml";"link_libozmqw"] ["libozmqw.a"];

    | _ -> ()
end
