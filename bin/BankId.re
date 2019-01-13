/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

let certPath = Fpath.v(Filename.dirname(Sys.argv[0]) ++ "/cert.crt");

let keyPath =
  Fpath.v(
    Filename.dirname(Sys.argv[0]) ++ "/FPTestcert2_20150818_102329.crt",
  );

let caPath =
  Fpath.v(Filename.dirname(Sys.argv[0]) ++ "/TestBankIDSSLRootCAv1Test.crt");

let testConfig = Lib.Config.tlsConfig(certPath, keyPath, caPath);

let logRes = res =>
  switch (res) {
  | Ok(body) => Logs.app(m => m("%s", body))
  | Error(x) => Logs.err(m => m("Oops! %s", Lib.Util.error_to_string(x)))
  };

let auth = pno => {
  let res =
    Lib.Util.sendReqWithConfig(testConfig, Lib.Auth.req(pno))
    |> Lwt_main.run
    |> (
      fun
      | Ok(body) => {
          let authResponse = Lib.Auth.parseResponse(body);
          Logs.app(m => m("orderRef: %s", authResponse.orderRef));
        }
      | Error(x) => Logs.err(m => m("Oops! %s", Lib.Util.error_to_string(x)))
    );
  ();
};

let collect = orderRef =>
  Lib.Util.sendReqWithConfig(testConfig, Lib.Collect.req(orderRef))
  |> Lwt_main.run
  |> logRes;

let cancel = orderRef =>
  Lib.Util.sendReqWithConfig(testConfig, Lib.Cancel.req(orderRef))
  |> Lwt_main.run
  |> logRes;

let authAndCollect = (pno): unit =>
  Lwt.Infix.(
    Lib.Auth.auth(testConfig, pno)
    >>= (
      r =>
        switch (r) {
        | Ok(body) =>
          let authResponse = Lib.Auth.parseResponse(body);
          Lib.Collect.collectUntilComplete(testConfig, authResponse.orderRef)
          >|= logRes;
        | Error(x) =>
          Lwt.return(
            Logs.err(m => m("Oops! %s", Lib.Util.error_to_string(x))),
          )
        }
    )
    |> Lwt_main.run
  );

let pno = {
  let doc = "Your personal number.";
  Cmdliner.Arg.(
    value & opt(string, "") & info(["p", "pno"], ~docv="PNO", ~doc)
  );
};

let orderRef = {
  let doc = "Your orderRef.";
  Cmdliner.Arg.(
    value & opt(string, "") & info(["o", "orderref"], ~docv="OREF", ~doc)
  );
};

let auth_t = Cmdliner.Term.(const(auth) $ pno, Cmdliner.Term.info("auth"));
let collect_t =
  Cmdliner.Term.(const(collect) $ orderRef, Cmdliner.Term.info("collect"));
let authAndCollect_t =
  Cmdliner.Term.(
    const(authAndCollect) $ pno,
    Cmdliner.Term.info("auth-and-collect"),
  );
let cancel_t =
  Cmdliner.Term.(const(cancel) $ orderRef, Cmdliner.Term.info("cancel"));

let () =
  Cmdliner.Term.exit @@
  Cmdliner.Term.eval_choice(auth_t, [collect_t, authAndCollect_t, cancel_t]);
