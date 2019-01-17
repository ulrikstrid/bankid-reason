let toOrderRef = json =>
  Yojson.Basic.Util.(json |> member("orderRef") |> to_string);

type pendingHint =
  | OutstandingTransaction
  | NoClient
  | Started
  | UserSign
  | Unknown;

let toPendingHint = json =>
  switch (
    Yojson.Basic.Util.member("hintCode", json) |> Yojson.Basic.Util.to_string
  ) {
  | "outstandingTransaction" => OutstandingTransaction
  | "noClient" => NoClient
  | "started" => Started
  | "userSign" => UserSign
  | _ => Unknown
  };

type failedHint =
  | ExpiredTransaction
  | CertificateErr
  | UserCancel
  | Cancelled
  | StartFailed
  | Unknown;

let toFailedHint = json =>
  switch (
    Yojson.Basic.Util.member("hintCode", json) |> Yojson.Basic.Util.to_string
  ) {
  | "expiredTransaction" => ExpiredTransaction
  | "certificateErr" => CertificateErr
  | "userCancel" => UserCancel
  | "cancelled" => Cancelled
  | "startFailed" => StartFailed
  | _ => Unknown
  };

type userData = {
  personalNumber: string,
  name: string,
  givenName: string,
  surname: string,
};

let toUserData = json =>
  Yojson.Basic.Util.{
    personalNumber: json |> member("personalNumber") |> to_string,
    name: json |> member("name") |> to_string,
    givenName: json |> member("givenName") |> to_string,
    surname: json |> member("surname") |> to_string,
  };

type certData = {
  notBefore: string,
  notAfter: string,
};

let toCertData = json =>
  Yojson.Basic.Util.{
    notBefore: json |> member("notBefore") |> to_string,
    notAfter: json |> member("notAfter") |> to_string,
  };

type completionData = {
  user: userData,
  cert: certData,
  deviceIp: string,
  signature: string,
  ocspResponse: string,
};

let toCompletionData = json =>
  Yojson.Basic.Util.(
    json
    |> member("completionData")
    |> (
      j => {
        user: j |> member("user") |> toUserData,
        cert: j |> member("cert") |> toCertData,
        deviceIp: j |> member("device") |> member("ipAddress") |> to_string,
        signature: j |> member("signature") |> to_string,
        ocspResponse: j |> member("ocspResponse") |> to_string,
      }
    )
  );

type collectResponse =
  | Pending((string, pendingHint))
  | Failed((string, failedHint))
  | Complete((string, completionData));

let parseResponse = str =>
  Yojson.Basic.from_string(str)
  |> (
    json => {
      let status =
        Yojson.Basic.Util.member("status", json)
        |> Yojson.Basic.Util.to_string;
      switch (status) {
      | "pending" => Pending((toOrderRef(json), toPendingHint(json)))
      | "failed" => Failed((toOrderRef(json), toFailedHint(json)))
      | "complete" => Complete((toOrderRef(json), toCompletionData(json)))
      };
    }
  );

let req = orderRef =>
  Uri.of_string("https://appapi2.test.bankid.com/rp/v5/collect")
  |> Httpkit.Client.Request.create(
       ~body={|{"orderRef":"|} ++ orderRef ++ {|"}|},
       ~headers=[("Content-Type", "application/json")],
       `POST,
     );

let collect = (config, orderRef) =>
  req(orderRef) |> Util.sendReqWithConfig(config);

let rec collectUntilComplete = (config, orderRef) =>
  Lwt.Infix.(
    collect(config, orderRef)
    >>= (
      fun
      | Ok(body) => {
          let res = parseResponse(body);
          switch (res) {
          | Complete((_, completionData)) => Ok(completionData) |> Lwt.return
          | Pending(_) =>
            Logs.app(m => m("status: pending"));
            Lwt_unix.sleep(2.)
            >>= (() => collectUntilComplete(config, orderRef));
          | Failed(_) => Lwt.return(Error(`Reading_error))
          };
        }
      | Error(x) => Error(x) |> Lwt.return
    )
  );
