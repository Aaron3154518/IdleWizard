#ifndef FIREBALL_HIT_H
#define FIREBALL_HIT_H

#include <ServiceSystem/Observable.h>

template <class T>
class FireballHitObservable : public ObservableCopy<T> {
   public:
    class TObservable : public Observable<T> {
        friend class FireballHitObservable<T>;
    };
    typedef typename TObservable::SubscriptionPtr FireballsSubscriptionPtr;

    FireballsSubscriptionPtr subscribeToFireballs(const T& t) {
        auto sub = mTObservable.subscribe(t);
        T& subObservable = sub->template get<0>();
        for (auto fireballSub : *this) {
            subObservable.subscribe(fireballSub);
        }
        return sub;
    }

   private:
    void onSubscribe(typename ObservableCopy<T>::SubscriptionPtr fireballSub) {
        for (auto sub : mTObservable) {
            sub->template get<0>().subscribe(fireballSub);
        }
    }

    TObservable mTObservable;
};

#endif
