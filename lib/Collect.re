let req = orderRef =>
  Uri.of_string("https://appapi2.test.bankid.com/rp/v5/collect")
  |> Httpkit.Client.Request.create(
       ~body={|{"orderRef":"|} ++ orderRef ++ {|"}|},
       ~headers=[("Content-Type", "application/json")],
       `POST,
     );

let collect = (config, orderRef) =>
  req(orderRef) |> Util.sendReqWithConfig(config);

type collectResponse = {
  orderRef: string,
  status: string,
  hintCode: string,
};

let parseResponse = json =>
  Yojson.Basic.from_string(json)
  |> (
    j =>
      Yojson.Basic.Util.{
        orderRef: j |> member("orderRef") |> to_string,
        status: j |> member("status") |> to_string,
        hintCode: j |> member("hintCode") |> to_string,
      }
  );

let rec collectUntilComplete = (config, orderRef) =>
  Lwt.Infix.(
    req(orderRef)
    |> Util.sendReqWithConfig(config)
    >>= (
      fun
      | Ok(body) => {
          let res = parseResponse(body);
          switch (res.status) {
          | "complete" => Ok(body) |> Lwt.return
          | status =>
            Logs.app(m =>
              m("status: %s\nhintCode: %s", res.status, res.hintCode)
            );
            Lwt_unix.sleep(2.)
            >>= (() => collectUntilComplete(config, orderRef));
          };
        }
      | Error(x) => Error(x) |> Lwt.return
    )
  );
