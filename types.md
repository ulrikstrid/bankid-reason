# Thinking of types

## Current BankID types

```ts
interface BankIdCollect {
  orderRef: string;
  status: "pending" | "failed" | "complete";
  hintCode:  // We have this when status is "pending"
    | "outstandingTransaction"
    | "noClient"
    | "started"
    | "userSign"
    // ...or "failed"
    | "expiredTransaction"
    | "certificateErr"
    | "userCancel"
    | "cancelled"
    | "startFailed"
    // This doesn't exist but others could be added
    | "unknown";

  // We have this when status is "complete"
  completionData?: {
    user: {
      personalNumber: string;
      name: string; // givenname + surname
      givenName: string;
      surname: string;
    };
    device: {
      ipAddress: string; // could be used to constrain ip
    };
    cert: {
      notBefore: string; // unix ms
      notAfter: string; // unix ms
    };
    signature: string; // Base64 XML?
    ocspResponse: string; // Base64 XML?
  };
}
```

## Wanted types

```ocaml
type pendingHint =
| OutstandingTransaction (* Tell them to start the app or that you are trying to do it *)
| NoClient (* The start failed, tell them to start the app *)
| Started (* User has started the app but not ID found yet *)
| UserSign (* The client has received the order *)
| Unknown (* New, unhandeled hint*)

type failedHint =
| ExpiredTransaction (* Did not complete in time *)
| CertificateErr (* The user failed to auth *)
| UserCancel (* User canceled the order *)
| Cancelled (* Order was canceled *)
| StartFailed (* The RP failed or the app is not instaled *)
| Unknown (* New, unhandeled hint*)

type user = {
  personalNumber: string;
  name: string; (* givenname + surname *)
  givenName: string;
  surname: string;
}

type cert = {
  notBefore: string (* unix ms *),
  notAfter: string (* unix ms *)
}

type completionData {
  user: user,
  cert: cert,
  deviceIp: string,
  signature: string, (* Base64 XML? *)
  ocspResponse: string (* Base64 *)
}

type Collect('user, 'c) =
| Pending((string, pendingHint))
| Complete((string, completionData))
| Failed((string, failedHint))
```
