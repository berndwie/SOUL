/*
    _____ _____ _____ __
   |   __|     |  |  |  |      The SOUL language
   |__   |  |  |  |  |  |__    Copyright (c) 2019 - ROLI Ltd.
   |_____|_____|_____|_____|

   The code in this file is provided under the terms of the ISC license:

   Permission to use, copy, modify, and/or distribute this software for any purpose
   with or without fee is hereby granted, provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

namespace soul
{

//==============================================================================
/**
    Abstract base class for a "performer" which can compile and execute a soul::Program.

    A typical performer is likely to be a JIT compiler or an interpreter.

    Note that performer implementations are not expected to be thread-safe!
    Performers will typically not create any internal threads, and all its methods
    are synchronous (for an asynchronous playback engine, see soul::Venue).
    Any code which uses a performer is responsible for making sure it calls the methods
    in a race-free way, and takes into account the fact that some of the calls may block
    for up to a few seconds.
*/
class Performer
{
public:
    virtual ~Performer() {}

    /** Provides the program for the performer to load.
        If a program is already loaded or linked, calling this should reset the state
        before attempting to load the new one.
        After successfully loading a program, the caller should then connect getter/setter
        callback to any endpoints that it wants to communicate with, and then call link()
        to prepare it for use.
        Note that this method blocks until building is finished, and it's not impossible
        that an optimising JIT engine could take up to several seconds, so make sure
        the caller takes this into account.
        Returns true on success; on failure, the CompileMessageList should contain error
        messages describing what went wrong.
    */
    virtual bool load (CompileMessageList&, const Program& programToLoad) = 0;

    /** Unloads any currently loaded program, and resets the state of the performer. */
    virtual void unload() = 0;

    /** When a program has been loaded (but not necessarily linked), this returns
        a list of the input endpoints that the program offers.
        Before linking, a caller will typically want to attach their callbacks to
        the
    */
    virtual std::vector<InputEndpoint::Ptr> getInputEndpoints() = 0;

    /** When a program has been loaded (but not necessarily linked), this returns
        a list of the output endpoints that the program offers.
    */
    virtual std::vector<OutputEndpoint::Ptr> getOutputEndpoints() = 0;

    /** After loading a program, and optionally connecting up to some of its endpoints,
        link() will complete any preparations needed before the code can be executed.
        If this returns true, then you can safely start calling advance(). If it
        returns false, the error messages will be added to the CompileMessageList
        provided.
        Note that this method blocks until building is finished, and it's not impossible
        that an optimising JIT engine could take up to several seconds, so make sure
        the caller takes this into account.
    */
    virtual bool link (CompileMessageList&, const LinkOptions&, LinkerCache*) = 0;

    /** Returns true if a program is currently loaded. */
    virtual bool isLoaded() = 0;

    /** Returns true if a program is successfully linked and ready to execute. */
    virtual bool isLinked() = 0;

    /** Resets the performer to the state it was in when freshly linked.
        This doesn't unlink or unload the program, it simply resets the program's
        internal state so that the next advance() call will begin a fresh run.
    */
    virtual void reset() = 0;

    /** Renders the next block of samples.
        Once a program has been loaded and linked, a caller will typically make repeated
        calls to advance() to actually perform the rendering work. During these calls, the
        performer will make whatever callbacks it needs to fill and empty its endpoint buffers,
        using the callbacks that the caller attached before linking.
        Because you're likely to be calling advance() from an audio thread, be careful not to
        allow any calls to other methods such as unload() to overlap with calls to advance()!
    */
    virtual void advance (uint32_t samplesToAdvance) = 0;

    /** Returns the number of over- or under-runs that have happened since the program was linked.
        Underruns can happen when an endpoint callback fails to empty or fill the amount of data
        that it is asked to handle.
    */
    virtual uint32_t getXRuns() = 0;
};


//==============================================================================
/**
    Abstract base class for a factory which can construct Performers
*/
class PerformerFactory
{
public:
    virtual ~PerformerFactory() {}

    virtual std::unique_ptr<Performer> createPerformer() = 0;
};

} // namespace soul
