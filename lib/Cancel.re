let req = orderRef =>
  Uri.of_string("https://appapi2.test.bankid.com/rp/v5/cancel")
  |> Httpkit.Client.Request.create(
       ~body={|{"orderRef":"|} ++ orderRef ++ {|"}|},
       ~headers=[("Content-Type", "application/json")],
       `POST,
     );

let auth = (config, pno) => req(pno) |> Util.sendReqWithConfig(config);
