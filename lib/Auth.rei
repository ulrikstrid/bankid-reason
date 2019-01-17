let req: string => Httpkit.Client.Request.t;

/** [auth config pno] starts a authentication for the passed [pno] against BankID using the passed in TLS [config] */
let auth:
  (Httpkit_lwt.Client.Https.config, string) =>
  Lwt.t(
    result(
      string,
      [>
        | `Connection_error(Httpaf.Client_connection.error)
        | `Reading_error
      ],
    ),
  );

type authResponse = {
  orderRef: string,
  autoStartToken: string,
};

let parseResponse: string => authResponse;
