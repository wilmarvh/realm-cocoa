////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>

#import "RLMSyncUtil.h"

@class RLMSyncSession, RLMSyncTimeoutOptions;

NS_ASSUME_NONNULL_BEGIN

/// An enum representing different levels of sync-related logging that can be configured.
typedef NS_ENUM(NSUInteger, RLMSyncLogLevel) {
    /// Nothing will ever be logged.
    RLMSyncLogLevelOff,
    /// Only fatal errors will be logged.
    RLMSyncLogLevelFatal,
    /// Only errors will be logged.
    RLMSyncLogLevelError,
    /// Warnings and errors will be logged.
    RLMSyncLogLevelWarn,
    /// Information about sync events will be logged. Fewer events will be logged in order to avoid overhead.
    RLMSyncLogLevelInfo,
    /// Information about sync events will be logged. More events will be logged than with `RLMSyncLogLevelInfo`.
    RLMSyncLogLevelDetail,
    /// Log information that can aid in debugging.
    ///
    /// - warning: Will incur a measurable performance impact.
    RLMSyncLogLevelDebug,
    /// Log information that can aid in debugging. More events will be logged than with `RLMSyncLogLevelDebug`.
    ///
    /// - warning: Will incur a measurable performance impact.
    RLMSyncLogLevelTrace,
    /// Log information that can aid in debugging. More events will be logged than with `RLMSyncLogLevelTrace`.
    ///
    /// - warning: Will incur a measurable performance impact.
    RLMSyncLogLevelAll
};

/// A log callback function which can be set on RLMSyncManager.
///
/// The log function may be called from multiple threads simultaneously, and is
/// responsible for performing its own synchronization if any is required.
typedef void (*RLMSyncLogFunction)(RLMSyncLogLevel level, NSString *message);

/// A block type representing a block which can be used to report a sync-related error to the application. If the error
/// pertains to a specific session, that session will also be passed into the block.
typedef void(^RLMSyncErrorReportingBlock)(NSError *, RLMSyncSession * _Nullable);

/**
 A singleton manager which serves as a central point for sync-related configuration.
 */
@interface RLMSyncManager : NSObject

/**
 A block which can optionally be set to report sync-related errors to your application.

 Any error reported through this block will be of the `RLMSyncError` type, and marked
 with the `RLMSyncErrorDomain` domain.

 Errors reported through this mechanism are fatal, with several exceptions. Please consult
 `RLMSyncError` for information about the types of errors that can be reported through
 the block, and for for suggestions on handling recoverable error codes.

 @see `RLMSyncError`
 */
@property (nullable, nonatomic, copy) RLMSyncErrorReportingBlock errorHandler;

/**
 A reverse-DNS string uniquely identifying this application. In most cases this
 is automatically set by the SDK, and does not have to be explicitly configured.
 */
@property (nonatomic, copy) NSString *appID;

/**
 A string identifying this application which is included in the User-Agent
 header of sync connections. By default, this will be the application's bundle
 identifier.

 This property must be set prior to opening a synchronized Realm for the first
 time. Any modifications made after opening a Realm will be ignored.
 */
@property (nonatomic, copy) NSString *userAgent;

/**
 The logging threshold which newly opened synced Realms will use. Defaults to
 `RLMSyncLogLevelInfo`.

 By default logging strings are output to Apple System Logger. Set `logger` to
 perform custom logging logic instead.

 @warning This property must be set before any synced Realms are opened. Setting it after
          opening any synced Realm will do nothing.
 */
@property (nonatomic) RLMSyncLogLevel logLevel;

/**
 The function which will be invoked whenever the sync client has a log message.

 If nil, log strings are output to Apple System Logger instead.

 @warning This property must be set before any synced Realms are opened. Setting
 it after opening any synced Realm will do nothing.
 */
@property (nonatomic, nullable) RLMSyncLogFunction logger;

/**
 The name of the HTTP header to send authorization data in when making requests to a Realm Object Server which has
 been configured to expect a custom authorization header.
 */
@property (nullable, nonatomic, copy) NSString *authorizationHeaderName;

/**
 Extra HTTP headers to append to every request to a Realm Object Server.
 */
@property (nullable, nonatomic, copy) NSDictionary<NSString *, NSString *> *customRequestHeaders;

/**
 A map of hostname to file URL for pinned certificates to use for HTTPS requests.

 When initiating a HTTPS connection to a server, if this dictionary contains an
 entry for the server's hostname, only the certificates stored in the file (or
 any certificates signed by it, if the file contains a CA cert) will be accepted
 when initiating a connection to a server. This prevents certain certain kinds
 of man-in-the-middle (MITM) attacks, and can also be used to trust a self-signed
 certificate which would otherwise be untrusted.

 On macOS, the certificate files may be in any of the formats supported by
 SecItemImport(), including PEM and .cer (see SecExternalFormat for a complete
 list of possible formats). On iOS and other platforms, only DER .cer files are
 supported.

 For example, to pin example.com to a .cer file included in your bundle:

 <pre>
 RLMSyncManager.sharedManager.pinnedCertificatePaths = @{
    @"example.com": [NSBundle.mainBundle pathForResource:@"example.com" ofType:@"cer"]
 };
 </pre>
 */
@property (nullable, nonatomic, copy) NSDictionary<NSString *, NSURL *> *pinnedCertificatePaths;

@property (nullable, atomic, copy) RLMSyncTimeoutOptions *timeoutOptions;

/// The sole instance of the singleton.
+ (instancetype)sharedManager NS_REFINED_FOR_SWIFT;

/// :nodoc:
- (instancetype)init __attribute__((unavailable("RLMSyncManager cannot be created directly")));

/// :nodoc:
+ (instancetype)new __attribute__((unavailable("RLMSyncManager cannot be created directly")));

@end

@interface RLMSyncTimeoutOptions : NSObject
/// The maximum number of milliseconds to allow for a connection to
/// become fully established. This includes the time to resolve the
/// network address, the TCP connect operation, the SSL handshake, and
/// the WebSocket handshake.
@property (nonatomic) NSUInteger connectTimeout;

/// The number of milliseconds to keep a connection open after all
/// sessions have been abandoned (or suspended by errors).
///
/// The purpose of this linger time is to avoid close/reopen cycles
/// during short periods of time where there are no sessions interested
/// in using the connection.
///
/// If the connection gets closed due to an error before the linger time
/// expires, the connection will be kept closed until there are sessions
/// willing to use it again.
@property (nonatomic) NSUInteger connectionLingerTime;

/// The client will send PING messages periodically to allow the server
/// to detect dead connections (heartbeat). This parameter specifies the
/// time, in milliseconds, between these PING messages.
@property (nonatomic) NSUInteger pingKeepalivePeriod;

/// Whenever the server receives a PING message, it is supposed to
/// respond with a PONG messsage to allow the client to detect dead
/// connections (heartbeat). This parameter specifies the time, in
/// milliseconds, that the client will wait for the PONG response
/// message before it assumes that the connection is dead, and
/// terminates it.
@property (nonatomic) NSUInteger pongKeepaliveTimeout;

/// The maximum amount of time, in milliseconds, since the loss of a
/// prior connection, for a new connection to be considered a *fast
/// reconnect*.
///
/// In general, when a client establishes a connection to the server,
/// the uploading process remains suspended until the initial
/// downloading process completes (as if by invocation of
/// Session::async_wait_for_download_completion()). However, to avoid
/// unnecessary latency in change propagation during ongoing
/// application-level activity, if the new connection is established
/// less than a certain amount of time (`fast_reconnect_limit`) since
/// the client was previously connected to the server, then the
/// uploading process will be activated immediately.
///
/// For now, the purpose of the general delaying of the activation of
/// the uploading process, is to increase the chance of multiple initial
/// transactions on the client-side, to be uploaded to, and processed by
/// the server as a single unit. In the longer run, the intention is
/// that the client should upload transformed (from reciprocal history),
/// rather than original changesets when applicable to reduce the need
/// for changeset to be transformed on both sides. The delaying of the
/// upload process will increase the number of cases where this is
/// possible.
@property (nonatomic) NSUInteger fastReconnectLimit;
@end

NS_ASSUME_NONNULL_END
