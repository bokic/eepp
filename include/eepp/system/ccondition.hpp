#ifndef EE_SYSTEMCCONDITION_HPP
#define EE_SYSTEMCCONDITION_HPP

#include <eepp/base.hpp>
#include <eepp/base/noncopyable.hpp>

namespace EE { namespace System {

namespace Platform { class cConditionImpl; }

/** @brief Blocks concurrent access to shared resources from multiple threads */
class EE_API cCondition : NonCopyable {
	public:
		//! Constants for arg 2 of WaitAndLock()
		enum LockType
		{
			ManualUnlock	= 0,	// false
			AutoUnlock		= 1		// true
		};

		/** Initializes a Condition object and sets its internal value to @value.
		 * Thus using WaitAndLock(@value, ...) will immediately return.
		 */
		cCondition( int value = 0 );
		
		/** Default destructor
		 * The Condition is invalidated before destruction
		 */
		~cCondition();
		
		void Lock();
		
		/** Waits until the Condition's value == awaitedValue and protects the Condition.
		 * You're responsible for unlocking the Condition with Unlock() after
		 * WaitAndLock() returned and after you're done working on protected data,
		 * or enabling the auto unlocking mechanism.
		 *
		 * The Condition locking guarantees that the condition remains true until
		 * you unlock it and that you are the only one that acquired the Condition.
		 *
		 * @awaitedValue: the value that should unlock the Condition
		 *
		 * @autoUnlock: Condition::AutoUnlock (true) to automatically unlock the Condition
		 * protection after it has been validated, or ManualUnlock (false) to
		 * manually choose when the Condition should be unlocked. While a Condition
		 * is locked, both WaitAndLock() and operator=() will block
		 * until the Condition is unlocked or invalidated. When a Condition is
		 * *automatically* unlocked, its value is not updated.
		 *
		 * @return: true if the @awaitedValue has been reached, false otherwise.
		 * WaitAndLock() may return even if @awaitedValue has not been
		 * reached if the Condition has been disabled through Invalidate(). An
		 * invalidated Condition always returns in an unlocked state.
		 */
		bool WaitAndLock( int awaitedValue, int autoUnlock = false );
		
		/** Unlocks a previously locked Condition with @value as
		 * internal value. When the condition is unlocked, it is assumed
		 * to have the given value. The condition is thereafter signaled.
		 * Unlocking a non-locked Condition is undefined.
		 *
		 * @value: the value the Condition should have when it is unlocked
		 */
		void Unlock( int value );
		
		void Unlock();
		
		/** Performs an assignement followed by a Signal() call.
		 * The internal Condition value is updated to @value and the Condition is
		 * signaled. Note that the Condition must be unlocked in order
		 * to be updated, otherwise it'll block until the Condition
		 * is unlocked.
		 *
		 * @value: the value to be assigned to the Condition
		 *
		 * @return: @value
		 */
		int operator=(int value);
		
		/** Get the current internal Condition value.
		 * This is a non-blocking call.
		 *
		 * @return: the current internal state
		 */
		int Value() const;
		
		/** Signals that the Condition state has changed and that
		 * threads waiting on this Condition should check
		 * the new internal value.
		 */
		void Signal();
		
		/** Signals the Condition and disables blocking calls,
		 * thus WaitAndLock() does no more wait whatever
		 * the awaitedValue is and waiting calls are unlocked, returning false.
		 */
		void Invalidate();
		
		/** Restores the blocking capabilities of the Condition,
		 * possibly previously disabled with Invalidate() 
		 */
		void Restore();
	protected:
		Platform::cConditionImpl *		mCondImpl;
};

}}

#endif